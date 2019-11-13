#pragma once
#include <array>
#include <memory>
#include <type_traits>
#include <seastar/util/log.hh>
#include "../Exceptions/LogicException.hpp"

namespace cpv {
	/** The storage used to store reusable objects for the specified type */
	template <class T, std::size_t Capacity_>
	struct ReusableStorage {
		// Notice: constructor is omitted to make compiler not generate tls init function
		// don't dynamic allocate this type
		static const constexpr std::size_t Capacity = Capacity_;
		std::array<std::unique_ptr<T>, Capacity> freeList;
		std::size_t size;
	};

	/** The storage capacity for the specified type, by default is 32kb / size */
	template <class T>
	static const constexpr std::size_t ReusableStorageCapacity = 32768 / sizeof(T);

	/** The storage type for the specified type */
	template <class T>
	using ReusableStorageType = ReusableStorage<T, ReusableStorageCapacity<T>>;

	/** The storage instance for the specified type */
	template <class T>
	extern thread_local ReusableStorageType<T> ReusableStorageInstance;

	/**
	 * Reusable unique pointer, back to free list automatically on destruction.
	 * T should provide two functions:
	 * - freeResources: called at deallocate
	 * - reset: called at allocate, with forwarded parameters
	 * And should define the storage in source file as:
	 * template <>
	 * thread_local ReusableStorageType<X> ReusableStorageInstance<X>;
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
		/** Constructor */
		explicit Reusable() noexcept :
			Reusable(nullptr, [](void*) noexcept {}) { }

		/** Constructor */
		explicit Reusable(std::unique_ptr<T>&& ptr) noexcept :
			Reusable(ptr.release(), [](void* ptr) noexcept {
				std::unique_ptr<T> tPtr(reinterpret_cast<T*>(ptr));
				try {
					auto& storage = ReusableStorageInstance<T>;
					std::size_t size = storage.size;
					if (size < ReusableStorageType<T>::Capacity) {
						tPtr->freeResources();
						storage.freeList[size] = std::move(tPtr);
						storage.size = size + 1;
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
			reset();
		}

		/** Delete pointer or move pointer to free list */
		void reset() {
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
		auto& storage = ReusableStorageInstance<T>;
		std::size_t size = storage.size;
		if (size == 0) {
			Reusable<T> object(std::make_unique<T>());
			object->reset(std::forward<Args>(args)...);
			return object;
		} else {
			Reusable<T> object(std::move(storage.freeList[--size]));
			storage.size = size;
			object->reset(std::forward<Args>(args)...);
			return object;
		}
	}
}

