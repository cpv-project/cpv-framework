#pragma once
#include <memory>
#include <array>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "../Utility/Macros.hpp"

namespace cpv {
	/**
	 * Allocator that allocate initial memory from stack (or the space contains this allocator).
	 * Notice this implementation is hacky and doesn't follow the standard allocator requirement.
	 *
	 * STL containers think allocators are just proxy types and the memory resource can be shared
	 * between multiple allocators, so memory allocated from allocator A can be deallocated from
	 * allocator B if A is copied to B or A is copied to C then to B, that make lifetime management
	 * of the memory resource much much harder, allocators should keep reference or shared pointer
	 * to the memory resource to arrive this requirement.
	 *
	 * For stack allocator share the storage with multiple allocators is just bad, sure I can make
	 * the storage external and require user to reserve it then pass to the allocator, but that will
	 * make copy or move container dangerous, and since some container will rebind the allocator for
	 * internal data type, the initial size is very hard to calculate, for example if you use 1024
	 * as initial size, it's not easy to calculate how many slots of hash table can reserve for
	 * an unordered map that won't trigger allocation from upstream allocator.
	 *
	 * Use this allocator for STL containers require override constructors and assignment operators,
	 * and should perform heavy tests to verify it works correctly.
	 */
	template <class T, std::size_t InitialSize, class UpstreamAllocator = std::allocator<T>>
	class StackAllocator : private UpstreamAllocator {
	private:
		using UpstreamAllocatorTrait = std::allocator_traits<UpstreamAllocator>;
		static_assert(std::is_same_v<T, typename UpstreamAllocatorTrait::value_type>);
		
	public:
		using value_type = T;
		template <class U>
		struct rebind {
			using other = StackAllocator<U, InitialSize,
				typename UpstreamAllocatorTrait::template rebind_alloc<U>>;
		};
		 
		/** Allocate memory */
		T* allocate(std::size_t n) {
			std::size_t index = index_;
			if (CPV_LIKELY(index + n <= InitialSize)) {
				// allocate from internal storage and increase counter
				T* ptr = reinterpret_cast<T*>(&storage_[index]);
				index_ = index + n;
				++count_;
				return ptr;
			} else {
				// allocate from upstream allocator
				return UpstreamAllocatorTrait::allocate(*this, n);
			}
		}
		
		/** Deallocate memory */
		void deallocate(T* ptr, std::size_t n) {
			if (CPV_LIKELY(
				static_cast<void*>(ptr) >= static_cast<void*>(storage_.begin()) &&
				static_cast<void*>(ptr) < static_cast<void*>(storage_.end()))) {
				// if all pointer allocated from initial storage deallocated, reset the index
				if (--count_ == 0) {
					index_ = 0;
				}
			} else {
				// deallocate from upstream allocator
				UpstreamAllocatorTrait::deallocate(*this, ptr, n);
			}
		}
		
		/** Copy constructor, copy nothing because the storage is fixed */
		template <class U, size_t UInitialSize, class UUpstreamAllocator>
		// cppcheck-suppress uninitMemberVar
		StackAllocator(const StackAllocator<U, UInitialSize, UUpstreamAllocator>&) : StackAllocator() { }
		// cppcheck-suppress uninitMemberVar
		StackAllocator(const StackAllocator&) : StackAllocator() { }
		
		/** Disable move consturctor */
		StackAllocator(StackAllocator&&) = delete;
		
		/** Disable copy assignment */
		StackAllocator& operator=(const StackAllocator&) = delete;
		
		/** Disable move assignment */
		StackAllocator& operator=(StackAllocator&&) = delete;
		
		/** Constructor */
		StackAllocator() :
			index_(0),
			count_(0) { }
		
	private:
		std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, InitialSize> storage_;
		std::size_t index_;
		std::size_t count_;
	};
	
	/** Compare StackAllocator, always return not equal */
	template <
		class T, std::size_t TInitialSize, class TUpstreamAllocator,
		class U, std::size_t UInitialSize, class UUpstreamAllocator>
	bool operator==(
		const StackAllocator<T, TInitialSize, TUpstreamAllocator>&,
		const StackAllocator<U, UInitialSize, UUpstreamAllocator>&) {
		return false;
	}
	template <
		class T, std::size_t TInitialSize, class TUpstreamAllocator,
		class U, std::size_t UInitialSize, class UUpstreamAllocator>
	bool operator!=(
		const StackAllocator<T, TInitialSize, TUpstreamAllocator>&,
		const StackAllocator<U, UInitialSize, UUpstreamAllocator>&) {
		return true;
	}
	
	/**
	 * std::vector uses StackAllocator, reserve initial storage automatically.
	 * Size comparison on 64 bit gcc:
	 * - std::vector<int>: 24
	 * - StackAllocatedVector<int, 1>: 48 (24 + 4 * 1 (storage) + 4 (padding) + 8 (index) + 8 (count))
	 * - StackAllocatedVector<int, 100>: 440 (24 + 4 * 100 (storage) + 8 (index) + 8 (count))
	 */
	template <
		class T,
		std::size_t InitialSize,
		class UpstreamAllocator = std::allocator<T>,
		class Allocator = StackAllocator<T, InitialSize, UpstreamAllocator>>
	class StackAllocatedVector : public std::vector<T, Allocator> {
	private:
		using Base = std::vector<T, Allocator>;
	public:
		StackAllocatedVector() { this->reserve(InitialSize); }
		StackAllocatedVector(const StackAllocatedVector& other) : Base(other) { }
		StackAllocatedVector(StackAllocatedVector&& other) : Base(std::move(other), Allocator()) { }
		// cppcheck-suppress noExplicitConstructor
		StackAllocatedVector(std::initializer_list<T> items) {
			this->reserve(std::max(InitialSize, items.size()));
			for (auto& item : items) {
				this->emplace_back(std::move(item));
			}
		}
		StackAllocatedVector& operator=(const StackAllocatedVector& other) {
			Base::operator=(other);
			return *this;
		}
		StackAllocatedVector& operator=(StackAllocatedVector&& other) {
			Base::operator=(std::move(other));
			return *this;
		}
	};
	
	/**
	 * std::unordered_map uses StackAllocator, reserve initial hash table automatically.
	 * - std::unordered_map<int, int>: 56
	 * - StackAllocatedUnorderedMap<int, int, 1>: 88 (56 + 16 (storage) + 8 (index) + 8 (count))
	 * - StackAllocatedUnorderedMap<int, int, 100>: 1672 (56 + 16 * 100 (storage) + 8 (index) + 8 (count))
	 */
	template <
		class Key,
		class T,
		std::size_t InitialSize,
		class Hash = std::hash<Key>,
		class KeyEqual = std::equal_to<Key>,
		class UpstreamAllocator = std::allocator<std::pair<const Key, T>>,
		class Allocator = StackAllocator<std::pair<const Key, T>, InitialSize, UpstreamAllocator>>
	class StackAllocatedUnorderedMap : public std::unordered_map<Key, T, Hash, KeyEqual, Allocator> {
	private:
		using Base = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;
	public:
		StackAllocatedUnorderedMap() { this->reserve(InitialSize); }
		StackAllocatedUnorderedMap(const StackAllocatedUnorderedMap& other) : Base() {
			this->reserve(std::max(InitialSize, other.size()));
			for (auto& item : other) {
				this->emplace(item);
			}
		}
		StackAllocatedUnorderedMap(StackAllocatedUnorderedMap&& other) : Base() {
			this->reserve(std::max(InitialSize, other.size()));
			for (auto& item : other) {
				this->emplace(std::move(item));
			}
			other.clear();
		}
		// cppcheck-suppress noExplicitConstructor
		StackAllocatedUnorderedMap(std::initializer_list<std::pair<const Key, T>> items) {
			this->reserve(std::max(InitialSize, items.size()));
			for (auto& item : items) {
				this->emplace(std::move(item));
			}
		}
		StackAllocatedUnorderedMap& operator=(const StackAllocatedUnorderedMap& other) {
			Base::operator=(other);
			return *this;
		}
		StackAllocatedUnorderedMap& operator=(StackAllocatedUnorderedMap&& other) {
			Base::operator=(std::move(other));
			return *this;
		}
	};
}

