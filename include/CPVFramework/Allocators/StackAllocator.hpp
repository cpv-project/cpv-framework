#pragma once
#include <memory>
#include <array>
#include <type_traits>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "../Utility/Macros.hpp"

namespace cpv {
	/**
	 * The memory storage used by StackAllocator<T>.
	 * 
	 * STL containers think allocators are just proxy types and the memory resource can be shared
	 * between multiple allocators, so memory allocated from allocator A can be deallocated from
	 * allocator B if A is copied to B or A is copied to C then to B, that make lifetime management
	 * of the memory resource much much harder, allocators should keep reference or shared pointer
	 * to the memory resource to arrive this requirement, which is sucks at all.
	 * 
	 * Since gcc 9 STL containers will not allow allocators store any useful information,
	 * all allocator must be stateless, otherwise you will face very very serious memory issue.
	 * (The standard tells you custom allocator can contains state since c++11, that's a lie).
	 * That's why this class exists, to use stack allocator you must manage the storage (the state)
	 * elsewhere, and care about the lifetime (copied container should not use old storage).
	 * You can't said I want to store first 100 elements on stack and later on heap, anymore.
	 * Because STL containers will rebind internal type and create new allocators,
	 * the T in allocator<T> is useless, the storage can't be type aware, such stupid design.
	 */
	template <std::size_t Size>
	class StackAllocatorStorage {
	public:
		/** Allocate memory from this storage, return nullptr if space not enough */
		template <class T>
		T* allocate(std::size_t n) {
			std::size_t size = sizeof(T) * n;
			void* ptr = std::align(alignof(T), size, ptr_, size_);
			if (CPV_LIKELY(ptr != nullptr)) {
				ptr_ = reinterpret_cast<char*>(ptr_) + size;
				size_ -= size;
				++allocated_count_;
			}
			return reinterpret_cast<T*>(ptr);
		}
		
		/** Check is pointer allocated from this storage */
		bool is_allocated_from_here(void* ptr) {
			return ptr >= buf_.data() && ptr < (buf_.data() + buf_.size());
		}
		
		/** Reduce allocated count, reuse storage when count reach to 0 */
		void deallocate() {
			--allocated_count_;
			if (allocated_count_ == 0) {
				ptr_ = buf_.data();
				size_ = buf_.size();
			}
		}
		
		/** Constructor, leave buf_ uninitialized */
		// cppcheck-suppress uninitMemberVar
		StackAllocatorStorage() :
			ptr_(buf_.data()),
			size_(buf_.size()),
			allocated_count_(0) { }
		
		/** Disallow copy and move construct */
		StackAllocatorStorage(const StackAllocatorStorage&) = delete;
		StackAllocatorStorage(StackAllocatorStorage&&) = delete;
		StackAllocatorStorage& operator=(const StackAllocatorStorage&) = delete;
		StackAllocatorStorage& operator=(StackAllocatorStorage&&) = delete;
		
	private:
		std::array<char, Size> buf_;
		void* ptr_;
		std::size_t size_;
		std::size_t allocated_count_;
	};
	
	/**
	 * Allocator that allocate initial memory from stack (or the space contains the storage),
	 * and following memory from upstream allocator.
	 *
	 * Use this allocator for STL containers require override constructors and assignment operators.
	 * You don't want STL containers copy allocator that uses old storage may destroy soon.
	 * And you must perform heavy tests to verify it works correctly.
	 */
	template <class T, std::size_t Size, class UpstreamAllocator = std::allocator<T>>
	class StackAllocator : private UpstreamAllocator {
	private:
		using UpstreamAllocatorTrait = std::allocator_traits<UpstreamAllocator>;
		static_assert(std::is_same_v<T, typename UpstreamAllocatorTrait::value_type>);
		
	public:
		using value_type = T;
		template <class U>
		struct rebind {
			using other = StackAllocator<U, Size,
				typename UpstreamAllocatorTrait::template rebind_alloc<U>>;
		};
		 
		/** Allocate memory */
		T* allocate(std::size_t n) {
			T* ptr = storage_.template allocate<T>(n);
			if (CPV_UNLIKELY(ptr == nullptr)) {
				ptr = UpstreamAllocatorTrait::allocate(*this, n);
			}
			return ptr;
		}
		
		/** Deallocate memory */
		void deallocate(T* ptr, std::size_t n) {
			if (CPV_LIKELY(storage_.is_allocated_from_here(ptr))) {
				storage_.deallocate();
			} else {
				UpstreamAllocatorTrait::deallocate(*this, ptr, n);
			}
		}
		
		/** Copy constructor for rebind */
		template <class U, class UUpstreamAllocator>
		StackAllocator(const StackAllocator<U, Size, UUpstreamAllocator>& other) :
			UpstreamAllocator(other), storage_(other.storage_) { }
		
		/** Copy assign operator for rebind */
		template <class U, class UUpstreamAllocator>
		StackAllocator& operator=(const StackAllocator<U, Size, UUpstreamAllocator>& other) {
			UpstreamAllocator::operator=(other);
			return *this;
		}
		
		/** Constructor */
		explicit StackAllocator(StackAllocatorStorage<Size>& storage) :
			storage_(storage) { }
		
	private:
		template <class U, std::size_t USize, class UUpstreamAllocator>
		friend class StackAllocator;
		
		StackAllocatorStorage<Size>& storage_;
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
	
	/** std::vector with StackAllocator, reserve initial storage automatically */
	template <
		class T,
		std::size_t InitialSize,
		class UpstreamAllocator = std::allocator<T>,
		std::size_t Size = sizeof(T)*InitialSize+alignof(T),
		class Allocator = StackAllocator<T, Size, UpstreamAllocator>>
	class StackAllocatedVector :
		private StackAllocatorStorage<Size>, // should initialize first
		public std::vector<T, Allocator> {
	private:
		using Storage = StackAllocatorStorage<Size>;
		using Base = std::vector<T, Allocator>;
	public:
		StackAllocatedVector() :
			Storage(), Base(Allocator(*this)) {
			this->reserve(InitialSize);
		}
		StackAllocatedVector(const StackAllocatedVector& other) :
			Storage(), Base(Allocator(*this)) {
			this->reserve(std::max(InitialSize, other.size()));
			for (auto& item : other) {
				this->emplace_back(item);
			}
		}
		StackAllocatedVector(StackAllocatedVector&& other) :
			Storage(), Base(Allocator(*this)) {
			this->reserve(std::max(InitialSize, other.size()));
			for (auto& item : other) {
				this->emplace_back(std::move(item));
			}
			other.clear();
		}
		// cppcheck-suppress noExplicitConstructor
		StackAllocatedVector(std::initializer_list<T> items) :
			Storage(), Base(Allocator(*this)) {
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
	
	/** std::unordered_map with StackAllocator, reserve initial hash table automatically */
	template <
		class Key,
		class T,
		std::size_t InitialSize,
		class Hash = std::hash<Key>,
		class KeyEqual = std::equal_to<Key>,
		class UpstreamAllocator = std::allocator<std::pair<const Key, T>>,
		std::size_t Size = InitialSize*(
			sizeof(typename std::unordered_map<Key, T>::node_type)+sizeof(std::uintptr_t))+
			alignof(typename std::unordered_map<Key, T>::node_type)+alignof(std::uintptr_t),
		class Allocator = StackAllocator<std::pair<const Key, T>, Size, UpstreamAllocator>>
	class StackAllocatedUnorderedMap :
		private StackAllocatorStorage<Size>, // should initialize first
		public std::unordered_map<Key, T, Hash, KeyEqual, Allocator> {
	private:
		using Storage = StackAllocatorStorage<Size>;
		using Base = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;
	public:
		StackAllocatedUnorderedMap() :
			Storage(), Base(Allocator(*this)) {
			this->reserve(InitialSize);
		}
		StackAllocatedUnorderedMap(const StackAllocatedUnorderedMap& other) :
			Storage(), Base(Allocator(*this)) {
			this->reserve(std::max(InitialSize, other.size()));
			for (auto& item : other) {
				this->emplace(item);
			}
		}
		StackAllocatedUnorderedMap(StackAllocatedUnorderedMap&& other) :
			Storage(), Base(Allocator(*this)) {
			this->reserve(std::max(InitialSize, other.size()));
			for (auto& item : other) {
				this->emplace(std::move(item));
			}
			other.clear();
		}
		// cppcheck-suppress noExplicitConstructor
		StackAllocatedUnorderedMap(std::initializer_list<std::pair<const Key, T>> items) :
			Storage(), Base(Allocator(*this)) {
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
		void clear() {
			// release hashtable to make allocated count in storage reach to 0
			Base empty(Allocator(*this));
			Base::operator=(std::move(empty));
			this->reserve(InitialSize);
		}
	};
	
	/** std::map with StackAllocator */
	template <
		class Key,
		class T,
		std::size_t InitialSize,
		class Compare = std::less<Key>,
		class UpstreamAllocator = std::allocator<std::pair<const Key, T>>,
		std::size_t Size = InitialSize*(
			sizeof(typename std::map<Key, T>::node_type)+sizeof(std::uintptr_t))+
			alignof(typename std::map<Key, T>::node_type)+alignof(std::uintptr_t),
		class Allocator = StackAllocator<std::pair<const Key, T>, Size, UpstreamAllocator>>
	class StackAllocatedMap :
		private StackAllocatorStorage<Size>, // should initialize first
		public std::map<Key, T, Compare, Allocator> {
	private:
		using Storage = StackAllocatorStorage<Size>;
		using Base = std::map<Key, T, Compare, Allocator>;
	public:
		StackAllocatedMap() :
			Storage(), Base(Allocator(*this)) { }
		StackAllocatedMap(const StackAllocatedMap& other) :
			Storage(), Base(Allocator(*this)) {
			for (auto& item : other) {
				this->emplace(item);
			}
		}
		StackAllocatedMap(StackAllocatedMap&& other) :
			Storage(), Base(Allocator(*this)) {
			for (auto& item : other) {
				this->emplace(std::move(item));
			}
			other.clear();
		}
		// cppcheck-suppress noExplicitConstructor
		StackAllocatedMap(std::initializer_list<std::pair<const Key, T>> items) :
			Storage(), Base(Allocator(*this)) {
			for (auto& item : items) {
				this->emplace(std::move(item));
			}
		}
		StackAllocatedMap& operator=(const StackAllocatedMap& other) {
			Base::operator=(other);
			return *this;
		}
		StackAllocatedMap& operator=(StackAllocatedMap&& other) {
			Base::operator=(std::move(other));
			return *this;
		}
	};
}

