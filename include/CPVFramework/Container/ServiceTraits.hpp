#pragma once
#include <vector>
#include <optional>
#include "../Allocators/StackAllocator.hpp"

namespace cpv {
	/** Determine attributes of a service type, like is collection or not */
	template <class T, class = void /* for enable_if */>
	struct ServiceTypeTrait {
		static const constexpr bool IsCollection = false;
		using Type = T;
		using ActualType = T;
	};
	
	/** For std::vector */
	template <class T>
	struct ServiceTypeTrait<std::vector<T>> {
		static const constexpr bool IsCollection = true;
		using Type = std::vector<T>;
		using ActualType = T;
		
		static void add(Type& collection, ActualType&& element) {
			collection.emplace_back(std::move(element));
		}
	};
	
	/** For StackAllocatedVector */
	template <class T, std::size_t InitialSize, class UpstreamAllocator, std::size_t Size, class Allocator>
	struct ServiceTypeTrait<StackAllocatedVector<
		T, InitialSize, UpstreamAllocator, Size, Allocator>> {
		static const constexpr bool IsCollection = true;
		using Type = StackAllocatedVector<T, InitialSize, UpstreamAllocator, Size, Allocator>;
		using ActualType = T;
		
		static void add(Type& collection, ActualType&& element) {
			collection.emplace_back(std::move(element));
		}
	};
	
	/** For std::optional, will store the last service instance if exists */
	template <class T>
	struct ServiceTypeTrait<std::optional<T>> {
		static const constexpr bool IsCollection = true;
		using Type = std::optional<T>;
		using ActualType = T;
		
		static void add(Type& collection, ActualType&& element) {
			collection.emplace(std::move(element));
		}
	};
	
	/** Get dependency types that should inject to the constructor, by default it's empty */
	template <class T, class = void>
	struct ServiceDependencyTrait {
		using DependencyTypes = std::tuple<>;
	};
	
	/** Get dependency types that should inject to the constructor, from T::DependencyTypes */
	template <class T>
	struct ServiceDependencyTrait<T, std::enable_if_t<(std::tuple_size_v<typename T::DependencyTypes> > 0)>> {
		// For example: std::tuple<A, B, C>;
		using DependencyTypes = typename T::DependencyTypes;
	};
}

