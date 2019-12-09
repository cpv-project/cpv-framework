#pragma once
#include <memory>
#include <optional>
#include <seastar/core/shared_ptr.hh>
#include "../Allocators/StackAllocator.hpp"
#include "./Reusable.hpp"

namespace cpv {
	/** Provide universe methods for creating, reseting and modifying objects */
	template <class T, class = void /* for enable_if */>
	struct ObjectTrait {
		static const constexpr bool IsPointerLike = false;
		static const constexpr bool IsCollectionLike = false;
		using Type = T;
		using UnderlyingType = T;
		template <class... Args>
		static inline Type create(Args&&... args) {
			return Type(std::forward<Args>(args)...);
		}
		static inline void reset(Type& value) { value = Type(); }
		static inline Type& get(Type& value) { return value; }
		static inline const Type& get(const Type& value) { return value; }
		template <class U>
		static inline void set(Type& target, U&& source) {
			target = std::forward<U>(source);
		}
	};

	/** Specialize for std::optional */
	template <class T>
	struct ObjectTrait<std::optional<T>> {
		static const constexpr bool IsPointerLike = true;
		static const constexpr bool IsCollectionLike = false;
		using Type = std::optional<T>;
		using UnderlyingType = T;
		template <class... Args>
		static inline Type create(Args&&... args) {
			std::optional<T> value;
			value.emplace(std::forward<Args>(args)...);
			return value;
		}
		static inline void reset(Type& value) { value.reset(); }
		static inline UnderlyingType* get(Type& value) {
			return value.has_value() ? value.operator->() : nullptr;
		}
		static inline const UnderlyingType* get(const Type& value) {
			return value.has_value() ? value.operator->() : nullptr;
		}
		template <class U>
		static inline void set(Type& target, U&& source) {
			if (target.has_value()) {
				*target = std::forward<U>(source);
			} else {
				target.emplace(std::forward<U>(source));
			}
		}
	};

	/** Specialize for std::unique_ptr */
	template <class T>
	struct ObjectTrait<std::unique_ptr<T>> {
		static const constexpr bool IsPointerLike = true;
		static const constexpr bool IsCollectionLike = false;
		using Type = std::unique_ptr<T>;
		using UnderlyingType = T;
		template <class... Args>
		static inline Type create(Args&&... args) {
			return std::make_unique<T>(std::forward<Args>(args)...);
		}
		static inline void reset(Type& value) { value.reset(); }
		static inline UnderlyingType* get(Type& value) { return value.get(); }
		static inline const UnderlyingType* get(const Type& value) { return value.get(); }
		template <class U>
		static inline void set(Type& target, U&& source) {
			if (target != nullptr) {
				*target = std::forward<U>(source);
			} else {
				target = create(std::forward<U>(source));
			}
		}
	};

	/** Specialize for seastar::shared_ptr */
	template <class T>
	struct ObjectTrait<seastar::shared_ptr<T>> {
		static const constexpr bool IsPointerLike = true;
		static const constexpr bool IsCollectionLike = false;
		using Type = seastar::shared_ptr<T>;
		using UnderlyingType = T;
		template <class... Args>
		static inline Type create(Args&&... args) {
			return seastar::make_shared<T>(std::forward<Args>(args)...);
		}
		static inline void reset(Type& value) { value = nullptr; }
		static inline UnderlyingType* get(Type& value) { return value.get(); }
		static inline const UnderlyingType* get(const Type& value) { return value.get(); }
		template <class U>
		static inline void set(Type& target, U&& source) {
			if (target != nullptr) {
				*target = std::forward<U>(source);
			} else {
				target = create(std::forward<U>(source));
			}
		}
	};

	/** Specialize for seastar::lw_shared_ptr */
	template <class T>
	struct ObjectTrait<seastar::lw_shared_ptr<T>> {
		static const constexpr bool IsPointerLike = true;
		static const constexpr bool IsCollectionLike = false;
		using Type = seastar::lw_shared_ptr<T>;
		using UnderlyingType = T;
		template <class... Args>
		static inline Type create(Args&&... args) {
			return seastar::make_lw_shared<T>(std::forward<Args>(args)...);
		}
		static inline void reset(Type& value) { value = nullptr; }
		static inline UnderlyingType* get(Type& value) { return value.get(); }
		static inline const UnderlyingType* get(const Type& value) { return value.get(); }
		template <class U>
		static inline void set(Type& target, U&& source) {
			if (target.get() != nullptr) {
				*target = std::forward<U>(source);
			} else {
				target = create(std::forward<U>(source));
			}
		}
	};

	/** Specialize for Reusable */
	template <class T>
	struct ObjectTrait<Reusable<T>> {
		static const constexpr bool IsPointerLike = true;
		static const constexpr bool IsCollectionLike = false;
		using Type = Reusable<T>;
		using UnderlyingType = T;
		template <class... Args>
		static inline Type create(Args&&... args) {
			return makeReusable<T>(std::forward<Args>(args)...);
		}
		static inline void reset(Type& value) { value.reset(); }
		static inline UnderlyingType* get(Type& value) { return value.get(); }
		static inline const UnderlyingType* get(const Type& value) { return value.get(); }
		template <class U>
		static inline void set(Type& target, U&& source) {
			target = makeReusable<T>(std::forward<U>(source));
		}
	};

	/** Specialize for std::vector */
	template <class T, class Allocator>
	struct ObjectTrait<std::vector<T, Allocator>> {
		static const constexpr bool IsPointerLike = false;
		static const constexpr bool IsCollectionLike = true;
		using Type = std::vector<T, Allocator>;
		using UnderlyingType = T;
		template <class... Args>
		static inline Type create(Args&&... args) {
			return Type(std::forward<Args>(args)...);
		}
		static inline void reset(Type& value) { value.clear(); }
		static inline Type& get(Type& value) { return value; }
		static inline const Type& get(const Type& value) { return value; }
		template <class U>
		static inline void set(Type& target, U&& source) { target = std::forward<U>(source); }
		static inline std::size_t size(const Type& values) {
			return values.size();
		}
		static inline void reserve(Type& values, std::size_t capacity) {
			values.reserve(capacity);
		}
		template <class... Args>
		static inline UnderlyingType& add(Type& values, Args&&... value) {
			return values.emplace_back(std::forward<Args>(value)...);
		}
		template <class Func>
		static inline void apply(const Type& values, const Func& func) {
			for (const auto& value : values) {
				func(value);
			}
		}
	};

	/** Specialize for StackAllocatedVector */
	template <class T, std::size_t InitialSize, class UpstreamAllocator>
	struct ObjectTrait<StackAllocatedVector<T, InitialSize, UpstreamAllocator>> {
		static const constexpr bool IsPointerLike = false;
		static const constexpr bool IsCollectionLike = true;
		using Type = StackAllocatedVector<T, InitialSize, UpstreamAllocator>;
		using UnderlyingType = T;
		template <class... Args>
		static inline Type create(Args&&... args) {
			return Type(std::forward<Args>(args)...);
		}
		static inline void reset(Type& value) { value.clear(); }
		static inline Type& get(Type& value) { return value; }
		static inline const Type& get(const Type& value) { return value; }
		template <class U>
		static inline void set(Type& target, U&& source) { target = std::forward<U>(source); }
		static inline std::size_t size(const Type& values) {
			return values.size();
		}
		static inline void reserve(Type& values, std::size_t capacity) {
			values.reserve(capacity);
		}
		template <class... Args>
		static inline UnderlyingType& add(Type& values, Args&&... value) {
			return values.emplace_back(std::forward<Args>(value)...);
		}
		template <class Func>
		static inline void apply(const Type& values, const Func& func) {
			for (const auto& value : values) {
				func(value);
			}
		}
	};
}

