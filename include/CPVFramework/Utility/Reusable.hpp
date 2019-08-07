#pragma once
#include <vector>
#include <memory>
#include <type_traits>
#include <seastar/util/log.hh>
#include "../Exceptions/LogicException.hpp"

namespace cpv {
	/** The storage used to store reusable objects for the specified type */
	template <class T>
	class ReusableStorage {
	public:
		/** Get the free object list */
		std::vector<std::unique_ptr<T>>& get() { return storage_; }

		/** Constructor */
		ReusableStorage(std::size_t capacity) : storage_() {
			storage_.reserve(capacity);
		}

	private:
		std::vector<std::unique_ptr<T>> storage_;
	};
	
	/**
	 * Reusable unique pointer, back to free list automatically on destruction.
	 * T should provide two functions:
	 * - freeResources: called at deallocate
	 * - reset: called at allocate, with forwarded parameters
	 * And should define the storage in source file as:
	 * // reuse up to 100 objects for each thread
	 * template <>
	 * thread_local ReusableStorage<X> Reusable<X>::Storage(100);
	 *
	 * Cast Reusable<Derived> to Reusable<Base> is supported (polymorphism is supported).
	 * Cast Reusable<Base> to Reusable<Derived> is also supported (use it carefully).
	 * Incomplete type is supported (however it require the complete definition on construct).
	 *
	 * Warning: Don't keep other Reusable<> live after freeResources, it may cause segment fault.
	 * - For example: A in freeList -> A refs B -> deallocate freeList B -> deallocate freeList A
	 */
	template <class T>
	class Reusable {
	public:
		/** The storage, should define for specified type in somewhere */
		static thread_local ReusableStorage<T> Storage;

		/** Constructor */
		explicit Reusable() noexcept :
			Reusable(nullptr, [](void*) noexcept {}) { }

		/** Constructor */
		explicit Reusable(std::unique_ptr<T>&& ptr) noexcept :
			Reusable(ptr.release(), [](void* ptr) noexcept {
				std::unique_ptr<T> tPtr(reinterpret_cast<T*>(ptr));
				try {
					auto& freeList = Storage.get();
					if (freeList.size() < freeList.capacity()) {
						tPtr->freeResources();
						freeList.emplace_back(std::move(tPtr));
					} else {
						tPtr.reset(); // call the derived destructor
					}
				} catch (...) {
					std::cerr << std::current_exception() << std::endl;
				}
			}) { }

		/** Move constructor */
		Reusable(Reusable&& other) noexcept :
			Reusable(other.ptr_, other.deleter_) {
			other.ptr_ = nullptr;
		}

		/** Move assignment */
		Reusable& operator=(Reusable&& other) noexcept {
			if (this != static_cast<void*>(&other)) {
				void* ptr = ptr_;
				if (ptr != nullptr) {
					ptr_ = nullptr;
					deleter_(ptr);
				}
				ptr_ = other.ptr_;
				deleter_ = other.deleter_;
				other.ptr_ = nullptr;
			}
			return *this;
		}

		/** Cast to another type */
		template <class U, std::enable_if_t<
			std::is_base_of<T, U>::value ||
			std::is_base_of<U, T>::value, int> = 0>
		Reusable<U> cast() && {
			if (CPV_UNLIKELY(reinterpret_cast<U*>(ptr_) !=
				static_cast<U*>(reinterpret_cast<T*>(ptr_)))) {
				// store the original pointer would solve this problem
				// but that will make Reusable to be 3 pointer size
				throw cpv::LogicException(CPV_CODEINFO,
					"cast cause pointer address changed, from",
					typeid(T).name(), "to", typeid(U).name());
			}
			void* ptr = ptr_;
			ptr_ = nullptr;
			return Reusable<U>(ptr, deleter_);
		}

		/** Disallow copy */
		Reusable(const Reusable&) = delete;
		Reusable& operator=(const Reusable&) = delete;

		/** Destructor */
		~Reusable() {
			void* ptr = ptr_;
			if (ptr != nullptr) {
				ptr_ = nullptr;
				deleter_(ptr);
			}
		}

		/** Dereference */
		T& operator*() const& {
			return *reinterpret_cast<T*>(ptr_);
		}

		/** Get pointer */
		T* operator->() const& {
			return reinterpret_cast<T*>(ptr_);
		}

		/** Get pointer */
		T* get() const& {
			return reinterpret_cast<T*>(ptr_);
		}

		/** Compare with nullptr */
		bool operator==(std::nullptr_t) const { return ptr_ == nullptr; }
		bool operator!=(std::nullptr_t) const { return ptr_ != nullptr; }

	private:
		/** Constructor */
		Reusable(void* ptr, void(*deleter)(void*) noexcept) noexcept :
			ptr_(ptr), deleter_(deleter) { }

		template <class> friend class Reusable;
		void* ptr_;
		void(*deleter_)(void*) noexcept;
	};

	/** Allocate reusable object */
	template <class T, class... Args>
	Reusable<T> makeReusable(Args&&... args) {
		auto& freeList = Reusable<T>::Storage.get();
		if (freeList.empty()) {
			Reusable<T> object(std::make_unique<T>());
			object->reset(std::forward<Args>(args)...);
			return object;
		} else {
			Reusable<T> object(std::move(freeList.back()));
			freeList.pop_back();
			object->reset(std::forward<Args>(args)...);
			return object;
		}
	}
}

