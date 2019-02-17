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
	 * Notice it doesn't support move construction, use it in stl containers will remove fast move support,
	 * and the move consturctor is deleted to avoid memory allocate from allocator A deallocate from B.
	 */
	template <class T, std::size_t InitialSize, class UpstreamAllocator = std::allocator<T>>
	class StackAllocator : private UpstreamAllocator {
	private:
		using UpstreamAllocatorTrait = std::allocator_traits<UpstreamAllocator>;
		
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
				T* ptr = reinterpret_cast<T*>(&storage_[index]);
				index_ = index + n;
				return ptr;
			} else {
				return UpstreamAllocatorTrait::allocate(*this, n);
			}
		}
		
		/** Deallocate memory */
		void deallocate(T* ptr, std::size_t n) {
			if (static_cast<void*>(ptr) < static_cast<void*>(storage_.begin()) ||
				static_cast<void*>(ptr) >= static_cast<void*>(storage_.end())) {
				return UpstreamAllocatorTrait::deallocate(*this, ptr, n);
			}
		}
		
		/** Copy constructor, copy nothing because the storage is fixed */
		template <class U, size_t UInitialSize, class UUpstreamAllocator>
		StackAllocator(const StackAllocator<U, UInitialSize, UUpstreamAllocator>&) : StackAllocator() { }
		StackAllocator(const StackAllocator&) : StackAllocator() { }
		
		/** Disable move consturctor */
		StackAllocator(StackAllocator&&) = delete;
		
		/** Constructor */
		StackAllocator() :
			storage_(),
			index_(0) { }
		
	private:
		std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, InitialSize> storage_;
		std::size_t index_;
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
	
	/** std::vector uses StackAllocator, reserve initial storage automatically */
	template <class T, std::size_t InitialSize, class UpstreamAllocator = std::allocator<T>>
	class StackAllocatedVector :
		public std::vector<T, StackAllocator<T, InitialSize, UpstreamAllocator>> {
	private:
		using Allocator = StackAllocator<T, InitialSize, UpstreamAllocator>;
		using Base = std::vector<T, Allocator>;
	public:
		StackAllocatedVector() { this->reserve(InitialSize); }
		StackAllocatedVector(const StackAllocatedVector& other) : Base(other) { }
		StackAllocatedVector(StackAllocatedVector&& other) : Base(std::move(other), Allocator()) { }
		StackAllocatedVector(std::initializer_list<T> items) {
			this->reserve(std::max(InitialSize, items.size()));
			for (auto& item : items) {
				this->emplace_back(std::move(item));
			}
		}
	};
	
	/** std::unordered_map uses StackAllocator, reserve initial hash table automatically */
	template <
		class Key,
		class T,
		std::size_t InitialSize,
		class Hash = std::hash<Key>,
		class KeyEqual = std::equal_to<Key>,
		class UpstreamAllocator = std::allocator<T>>
	class StackAllocatedUnorderedMap : public std::unordered_map<
		Key, T, Hash, KeyEqual,
		StackAllocator<std::pair<const Key, T>, InitialSize, UpstreamAllocator>> {
	private:
		using Allocator = StackAllocator<std::pair<const Key, T>, InitialSize, UpstreamAllocator>;
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
		StackAllocatedUnorderedMap(std::initializer_list<std::pair<const Key, T>> items) {
			this->reserve(std::max(InitialSize, items.size()));
			for (auto& item : items) {
				this->emplace(std::move(item));
			}
		}
	};
}

