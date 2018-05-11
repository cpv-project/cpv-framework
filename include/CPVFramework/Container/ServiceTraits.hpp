#pragma once
#include <memory>
#include <core/shared_ptr.hh>
#include "../Utility/Object.hpp"

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
	template <class TService, class TImplementation, class = void>
	struct ServiceFactoryTrait {
		TService operator()(const Container& container) const {
			if constexpr (std::is_constructible_v<TImplementation, const Container&>) {
				return static_cast<TService>(TImplementation(container));
			} else if constexpr (std::is_constructible_v<TImplementation>) {
				return static_cast<TService>(TImplementation());
			} else {
				static_assert(std::is_same_v<TImplementation, void>,
					"doesn't know how to construct implementation instance, "
					"please provide a constructor takes 'const Container&'");
			}
		}
	};

	/** For seastar::shared_ptr */
	template <class TServiceInner, class TImplementationInner>
	struct ServiceFactoryTrait<
		seastar::shared_ptr<TServiceInner>,
		seastar::shared_ptr<TImplementationInner>> {
		seastar::shared_ptr<TServiceInner> operator()(const Container& container) const {
			if constexpr (std::is_constructible_v<TImplementationInner, const Container&>) {
				return seastar::make_shared<TImplementationInner>(container);
			} else if constexpr (std::is_constructible_v<TImplementationInner>) {
				return seastar::make_shared<TImplementationInner>();
			} else {
				static_assert(std::is_same_v<TImplementationInner, void>,
					"doesn't know how to construct implementation instance, "
					"please provide a constructor takes 'const Container&'");
			}
		}
	};

	/** For seastar::lw_shared_ptr */
	template <class TServiceInner, class TImplementationInner>
	struct ServiceFactoryTrait<
		seastar::lw_shared_ptr<TServiceInner>,
		seastar::lw_shared_ptr<TImplementationInner>> {
		seastar::lw_shared_ptr<TServiceInner> operator()(const Container& container) const {
			static_assert(std::is_same_v<TServiceInner, TImplementationInner>,
				"seastar::lw_shared_ptr doesn't support polymorphism");
			if constexpr (std::is_constructible_v<TImplementationInner, const Container&>) {
				return seastar::make_lw_shared<TImplementationInner>(container);
			} else if constexpr (std::is_constructible_v<TImplementationInner>) {
				return seastar::make_lw_shared<TImplementationInner>();
			} else {
				static_assert(std::is_same_v<TImplementationInner, void>,
					"doesn't know how to construct implementation instance, "
					"please provide a constructor takes 'const Container&'");
			}
		}
	};

	/** For std::unique_ptr */
	template <class TServiceInner, class TImplementationInner>
	struct ServiceFactoryTrait<
		std::unique_ptr<TServiceInner>,
		std::unique_ptr<TImplementationInner>> {
		std::unique_ptr<TServiceInner> operator()(const Container& container) const {
			static_assert(std::is_same_v<TServiceInner, TImplementationInner>,
				"std::unique_ptr doesn't support polymorphism");
			if constexpr (std::is_constructible_v<TImplementationInner, const Container&>) {
				return std::make_unique<TImplementationInner>(container);
			} else if constexpr (std::is_constructible_v<TImplementationInner>) {
				return std::make_unique<TImplementationInner>();
			} else {
				static_assert(std::is_same_v<TImplementationInner, void>,
					"doesn't know how to construct implementation instance, "
					"please provide a constructor takes 'const Container&'");
			}
		}
	};

	/** For std::shared_ptr */
	template <class TServiceInner, class TImplementationInner>
	struct ServiceFactoryTrait<
		std::shared_ptr<TServiceInner>,
		std::shared_ptr<TImplementationInner>> {
		std::shared_ptr<TServiceInner> operator()(const Container& container) const {
			if constexpr (std::is_constructible_v<TImplementationInner, const Container&>) {
				return std::make_shared<TImplementationInner>(container);
			} else if constexpr (std::is_constructible_v<TImplementationInner>) {
				return std::make_shared<TImplementationInner>();
			} else {
				static_assert(std::is_same_v<TImplementationInner, void>,
					"doesn't know how to construct implementation instance, "
					"please provide a constructor takes 'const Container&'");
			}
		}
	};

	/** For Object */
	template <class TServiceInner, class TImplementationInner>
	struct ServiceFactoryTrait<
		Object<TServiceInner>,
		Object<TImplementationInner>> {
		Object<TServiceInner> operator()(const Container& container) const {
			using ResetType = decltype(&TImplementationInner::reset);
			if constexpr (
				std::is_invocable_v<ResetType, const Container&> || // static
				std::is_invocable_v<ResetType, TImplementationInner&, const Container&>) {
				return makeObject<TImplementationInner>(container).template cast<TServiceInner>();
			} else if constexpr (
				std::is_invocable_v<ResetType> || // static
				std::is_invocable_v<ResetType, TImplementationInner&>) {
				return makeObject<TImplementationInner>().template cast<TServiceInner>();
			} else {
				static_assert(std::is_same_v<TImplementationInner, void>,
					"doesn't know how to construct implementation instance, "
					"please provide a constructor takes 'const Container&'");
			}
		}
	};
}

