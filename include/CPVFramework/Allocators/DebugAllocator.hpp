#pragma once
#include <iostream>

namespace cpv {
	/** Modes for debug allocator */
	enum class DebugAllocatorMode {
		LogToStdout,
		NoLogging
	};
	
	/** Allocator that prints debug message and forward all requests to upstream allocator */
	template <
		class T,
		DebugAllocatorMode Mode = DebugAllocatorMode::LogToStdout,
		class UpstreamAllocator = std::allocator<T>>
	class DebugAllocator : public UpstreamAllocator {
	private:
		using UpstreamAllocatorTrait = std::allocator_traits<UpstreamAllocator>;
		static_assert(std::is_same_v<T, typename UpstreamAllocatorTrait::value_type>);
		
	public:
		template <class U>
		struct rebind {
			using other = DebugAllocator<U, Mode,
				typename UpstreamAllocatorTrait::template rebind_alloc<U>>;
		};
		
		/** Allocate memory */
		T* allocate(std::size_t n) {
			T* ptr = UpstreamAllocatorTrait::allocate(*this, n);
			if constexpr (Mode != DebugAllocatorMode::NoLogging) {
				std::cout << "[DebugAllocator] allocate " <<
					typeid(T).name() << " x " << n <<
					" from allocator " << this << ": " << ptr << std::endl;
			}
			return ptr;
		}
		
		/** Deallocate memory */
		void deallocate(T* ptr, std::size_t n) {
			if constexpr (Mode != DebugAllocatorMode::NoLogging) {
				std::cout << "[DebugAllocator] deallocate " <<
					typeid(T).name() << " x " << n <<
					" from allocator " << this << ": " << ptr << std::endl;
			}
			UpstreamAllocatorTrait::deallocate(*this, ptr, n);
		}
		
		/** Copy constructor */
		template <class U, DebugAllocatorMode UMode, class UUpstreamAllocator>
		DebugAllocator(const DebugAllocator<U, UMode, UUpstreamAllocator>& other) :
			UpstreamAllocator(other) {
			if constexpr (Mode != DebugAllocatorMode::NoLogging) {
				std::cout << "[DebugAllocator] template copy construct from " <<
					&other << " to " << this << std::endl;
			}
		}
		DebugAllocator(const DebugAllocator& other) : UpstreamAllocator(other) {
			if constexpr (Mode != DebugAllocatorMode::NoLogging) {
				std::cout << "[DebugAllocator] copy construct from " <<
					&other << " to " << this << std::endl;
			}
		}
		
		/** Move consturctor */
		DebugAllocator(DebugAllocator&& other) : UpstreamAllocator(std::move(other)) {
			if constexpr (Mode != DebugAllocatorMode::NoLogging) {
				std::cout << "[DebugAllocator] move construct from " <<
					&other << " to " << this << std::endl;
			}
		}
		
		/** Copy assignment */
		DebugAllocator& operator=(const DebugAllocator& other) {
			if constexpr (Mode != DebugAllocatorMode::NoLogging) {
				std::cout << "[DebugAllocator] copy assign from " <<
					&other << " to " << this << std::endl;
			}
			UpstreamAllocator::operator=(other);
			return *this;
		}
		
		/** Move assignment */
		DebugAllocator& operator=(DebugAllocator&& other) {
			if constexpr (Mode != DebugAllocatorMode::NoLogging) {
				std::cout << "[DebugAllocator] move assign from " <<
					&other << " to " << this << std::endl;
			}
			UpstreamAllocator::operator=(std::move(other));
			return *this;
		}
		
		/** Constructor */
		template <class... Args>
		DebugAllocator(Args&&... args) :
			UpstreamAllocator(std::forward<Args>(args)...) {
			if constexpr (Mode != DebugAllocatorMode::NoLogging) {
				std::cout << "[DebugAllocator] construct " << this << std::endl;
			}
		}
	};
}

