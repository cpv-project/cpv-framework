#pragma once
#include <vector>
#include <memory>
#include <type_traits>
#include <seastar/util/log.hh>
#include "../Exceptions/LogicException.hpp"

namespace cpv {
	/** Class used to determinate the free list size of the specified type */
	template <class T>
	struct ObjectFreeListSize {
		// Up to 32k per type
		static const constexpr std::size_t value = 32768/sizeof(T);
	};

	/**
	 * Reuseable unique pointer, back to free list automatically on destruction.
	 * T should provide two functions:
	 * - freeResources: called at deallocate
	 * - reset: called at allocate, with forwarded parameters
	 * Cast Object<Derived> to Object<Base> is supported (polymorphism is supported).
	 * Cast Object<Base> to Object<Derived> is also supported (use it carefully).
	 * Incomplete type is supported (however it require the complete definition on construct).
	 */
	template <class T>
	class Object {
	public:
		/** Constructor */
		explicit Object() noexcept :
			Object(nullptr, [](void*) noexcept {}) { }

		/** Constructor */
		explicit Object(std::unique_ptr<T>&& ptr) noexcept :
			Object(ptr.release(), [](void* ptr) noexcept {
				std::unique_ptr<T> tPtr(reinterpret_cast<T*>(ptr));
				try {
					auto& freeList = getFreeList();
					if (freeList.size() < ObjectFreeListSize<T>::value) {
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
		Object(Object&& other) noexcept :
			Object(other.ptr_, other.deleter_) {
			other.ptr_ = nullptr;
		}

		/** Move assignment */
		Object& operator=(Object&& other) noexcept {
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
		Object<U> cast() && {
			if (static_cast<U*>(reinterpret_cast<T*>(1)) != reinterpret_cast<U*>(1)) {
				// store the original pointer would solve this problem
				// but that will make Object to be 3 pointer size
				throw cpv::LogicException(CPV_CODEINFO,
					"cast cause pointer address changed, from",
					typeid(T).name(), "to", typeid(U).name());
			}
			void* ptr = ptr_;
			ptr_ = nullptr;
			return Object<U>(ptr, deleter_);
		}

		/** Disallow copy */
		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;

		/** Destructor */
		~Object() {
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

		/** Get the thread local storage of objects */
		static std::vector<std::unique_ptr<T>>& getFreeList() {
			static thread_local std::vector<std::unique_ptr<T>> freeList;
			return freeList;
		}

	private:
		/** Constructor */
		Object(void* ptr, void(*deleter)(void*) noexcept) noexcept :
			ptr_(ptr), deleter_(deleter) { }

		template <class> friend class Object;
		void* ptr_;
		void(*deleter_)(void*) noexcept;
	};

	/** Allocate object */
	template <class T, class... Args>
	Object<T> makeObject(Args&&... args) {
		auto& freeList = Object<T>::getFreeList();
		if (freeList.empty()) {
			Object<T> object(std::make_unique<T>());
			object->reset(std::forward<Args>(args)...);
			return object;
		} else {
			Object<T> object(std::move(freeList.back()));
			freeList.pop_back();
			object->reset(std::forward<Args>(args)...);
			return object;
		}
	}
}

