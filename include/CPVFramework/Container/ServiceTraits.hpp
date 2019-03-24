#pragma once
#include <vector>
#include "../Allocators/StackAllocator.hpp"

namespace cpv {
	// `class = void` is for std::enable_if_t
	
	/** Determine whether should return a single instance or multiple instances */
	template <class T, class = void>
	struct ServiceCollectionTrait {
		static const constexpr bool IsCollection = false;
		using Type = T;
	};
	
	/** For std::vector */
	template <class T>
	struct ServiceCollectionTrait<std::vector<T>> {
		static const constexpr bool IsCollection = true;
		using Type = std::vector<T>;
		using ElementType = T;
		
		static void add(T& collection, ElementType&& element) {
			collection.emplace_back(std::move(element));
		}
	};
	
	/** For StackAllocatedVector */
	template <class T, std::size_t InitialSize, class UpstreamAllocator, class Allocator>
	struct ServiceCollectionTrait<StackAllocatedVector<
		T, InitialSize, UpstreamAllocator, Allocator>> {
		static const constexpr bool IsCollection = true;
		using Type = StackAllocatedVector<T, InitialSize, UpstreamAllocator, Allocator>;
		using ElementType = T;
		
		static void add(Type& collection, ElementType&& element) {
			collection.emplace_back(std::move(element));
		}
	};
	
	/** Get dependency types that should inject to the constructor */
	template <class T, class = void>
	struct ServiceDependencyTrait {
		// For example: std::tuple<A, B, C>;
		using DependencyTypes = typename T::DependencyTypes;
	};
}

