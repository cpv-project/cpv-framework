#pragma once

namespace cpv {
	/** Specific whether should return a single instance or multiple instances */
	template <class TService, class = void>
	struct ServiceCollectionTrait {
		static const constexpr bool isCollection = false;
	};

	/** For std::vector */
	template <class TElementType>
	struct ServiceCollectionTrait<std::vector<TElementType>> {
		static const constexpr bool isCollection = true;
		using ElementType = TElementType;

		static void add(std::vector<TElementType>& collection, TElementType&& element) {
			collection.emplace_back(std::move(element));
		}
	};

	/** Specific how to construct the instance */
	template <class TImplementation, class = void>
	struct ServiceFactoryTrait {
		TImplementation operator()(const Container& container) const {
			if constexpr (std::is_constructible_v<TImplementation, const Container&>) {
				return TImplementation(container);
			} else if constexpr (std::is_constructible_v<TImplementation>) {
				return TImplementation();
			} else {
				static_assert(std::is_same_v<TImplementation, void>,
					"doesn't know how to construct implementation instance, "
					"please provide a constructor takes 'const Container&'");
			}
		}
	};

	/** For seastar::shared_ptr */
	template <class TElementType>
	struct ServiceFactoryTrait<seastar::shared_ptr<TElementType>> {
		seastar::shared_ptr<TElementType> operator()(const Container& container) const {
			if constexpr (std::is_constructible_v<TElementType, const Container&>) {
				return seastar::make_shared<TElementType>(container);
			} else if constexpr (std::is_constructible_v<TElementType>) {
				return seastar::make_shared<TElementType>();
			} else {
				static_assert(std::is_same_v<TElementType, void>,
					"doesn't know how to construct implementation instance, "
					"please provide a constructor takes 'const Container&'");
			}
		}
	};
}

