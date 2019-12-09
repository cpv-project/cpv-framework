#pragma once
#include <vector>
#include <optional>
#include "../Allocators/StackAllocator.hpp"
#include "../Utility/ObjectTrait.hpp"

namespace cpv {
	/** Determine attributes of a service type, like is collection or not */
	template <class T, class = void /* for enable_if */>
	struct ServiceTypeTrait {
		static const constexpr bool IsCollection = false;
		using Type = T;
		using ActualType = T;
	};
	
	/** Specialize for collection like types */
	template <class T>
	struct ServiceTypeTrait<T, std::enable_if_t<ObjectTrait<T>::IsCollectionLike>> {
		static const constexpr bool IsCollection = true;
		using Type = T;
		using ActualType = typename ObjectTrait<T>::UnderlyingType;
		
		static void add(Type& collection, ActualType&& element) {
			ObjectTrait<T>::add(collection, std::move(element));
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

