#pragma once
#include <chrono>
#include <memory>
#include <optional>
#include <seastar/core/shared_ptr.hh>
#include "../Allocators/StackAllocator.hpp"
#include "../Exceptions/DeserializeException.hpp"
#include "../Utility/ObjectTrait.hpp"
#include "../Utility/SharedString.hpp"
#include "./JsonDeserializer.sajson.hpp"

namespace cpv {
	// alias of sajson types
	using JsonType = sajson::type;
	using JsonDocument = sajson::document;

	/** Combine sajson::value and original json string */
	struct JsonValue : public sajson::value {
	public:
		using sajson::value::value;

		/** Constructor */
		JsonValue(const sajson::value& value, const SharedString& str) :
			sajson::value(value), str_(str) { }

		/** Get original json string */
		const SharedString& jsonStr() const& { return str_; }

		/** Get value by key for object, return TYPE_NOKEY if key not exists */
		JsonValue operator[](std::string_view key) const {
			return { get_value_of_key(sajson::string(key.data(), key.size())), str_ };
		}

		/** Get value by index for array, use out of range index is undefined behavior */
		JsonValue operator[](std::size_t index) const {
			return { get_array_element(index), str_ };
		}

	private:
		const SharedString& str_;
	};

	/**
	 * The class used to convert json value to model.
	 *
	 * The model type should contains a public function named loadJson
	 * that takes a JsonValue represents the object and return whether
	 * loaded successfully.
	 *
	 * You can specialize it for more types.
	 */
	template <class T, class = void /* for enable_if */>
	struct JsonValueConverter {
		/** Convert json value to model */
		static bool convert(T& model, const JsonValue& value) {
			if (CPV_LIKELY(value.get_type() == JsonType::TYPE_OBJECT)) {
				return model.loadJson(value);
			}
			return false;
		}
	};

	/** Specialize for integer */
	template <class T>
	struct JsonValueConverter<T,
		std::enable_if_t<std::numeric_limits<T>::is_integer && !std::is_same_v<T, bool>>> {
		/** Convert json value to integer if type matched */
		static bool convert(T& target, const JsonValue& value) {
			if (CPV_LIKELY(value.get_type() == JsonType::TYPE_INTEGER)) {
				target = static_cast<T>(value.get_integer_value());
				return true;
			}
			return false;
		}
	};

	/** Specialize for floating point */
	template <class T>
	struct JsonValueConverter<T,
		std::enable_if_t<std::is_floating_point_v<T>>> {
		/** Convert json value to double if type matched */
		static bool convert(T& target, const JsonValue& value) {
			if (value.get_type() == JsonType::TYPE_INTEGER) {
				target = static_cast<T>(value.get_integer_value());
				return true;
			} else if (value.get_type() == JsonType::TYPE_DOUBLE) {
				target = static_cast<T>(value.get_double_value());
				return true;
			}
			return false;
		}
	};

	/** Specialize for bool */
	template <>
	struct JsonValueConverter<bool> {
		/** Convert json value to bool if type matched */
		static bool convert(bool& target, const JsonValue& value) {
			if (value.get_type() == JsonType::TYPE_TRUE) {
				target = true;
				return true;
			} else if (value.get_type() == JsonType::TYPE_FALSE) {
				target = false;
				return true;
			}
			return false;
		}
	};

	/** Specialize for SharedString */
	template <>
	struct JsonValueConverter<SharedString> {
		/** Convert json value to SharedString if type matched */
		static bool convert(SharedString& target, const JsonValue& value) {
			if (CPV_LIKELY(value.get_type() == JsonType::TYPE_STRING)) {
				target = value.jsonStr().share();
				target.trim(std::string_view(
					value.as_cstring(), value.get_string_length()));
				return true;
			}
			return false;
		}
	};

	/** Specialize for std::string */
	template <>
	struct JsonValueConverter<std::string> {
		/** Convert json value to std::string if type matched */
		static bool convert(std::string& target, const JsonValue& value) {
			if (CPV_LIKELY(value.get_type() == JsonType::TYPE_STRING)) {
				target.assign(value.as_cstring(), value.get_string_length());
				return true;
			}
			return false;
		}
	};

	/** Specialize for collection like types */
	template <class T>
	struct JsonValueConverter<T, std::enable_if_t<
		ObjectTrait<T>::IsCollectionLike && !ObjectTrait<T>::IsPointerLike>> {
		/** Convert json value to collection like object if type matched */
		static bool convert(T& models, const JsonValue& value) {
			using Trait = ObjectTrait<T>;
			using UnderlyingType = typename Trait::UnderlyingType;
			if (CPV_LIKELY(value.get_type() == JsonType::TYPE_ARRAY)) {
				std::size_t length = value.get_length();
				Trait::reserve(models, Trait::size(models) + length);
				bool result = true;
				for (std::size_t i = 0; i < length; ++i) {
					result = JsonValueConverter<UnderlyingType>::convert(
						Trait::add(models), value[i]) && result;
				}
				return result;
			}
			return false;
		}
	};

	/** Specialize for pointer like types */
	template <class T>
	struct JsonValueConverter<T, std::enable_if_t<ObjectTrait<T>::IsPointerLike>> {
		/** Convert json value to pointer like object if type matched */
		static bool convert(T& target, const JsonValue& value) {
			using Trait = ObjectTrait<T>;
			using UnderlyingType = typename Trait::UnderlyingType;
			if (value.get_type() == JsonType::TYPE_NULL) {
				Trait::reset(target);
				return true;
			} else {
				if (Trait::get(target) == nullptr) {
					target = Trait::create();
				}
				return JsonValueConverter<UnderlyingType>::convert(
					*Trait::get(target), value);
			}
		}
	};

	/** Specialize for chrono durations */
	template <class Rep, class Period>
	struct JsonValueConverter<std::chrono::duration<Rep, Period>> {
		/** Convert json value to chrono duration if type matched */
		static bool convert(
			std::chrono::duration<Rep, Period>& target, const JsonValue& value) {
			if (CPV_LIKELY(value.get_type() == JsonType::TYPE_INTEGER)) {
				target = std::chrono::duration<Rep, Period>(
					static_cast<Rep>(value.get_integer_value()));
				return true;
			}
			return false;
		}
	};

	/** Convenient static function for JsonValueConverter */
	template <class T>
	static inline bool convertJsonValue(T& model, const JsonValue& value) {
		return JsonValueConverter<T>::convert(model, value);
	}

	/**
	 * The class used to deserialize json to model.
	 * 
	 * The model should be convertible from JsonValue by using JsonValueConverter.
	 *
	 * Since it uses sajson, and sajson is a in-situ parser, the json string
	 * must be mutable so encoded strings can be replaced by decoded strings.
	 * And since the contents of json string can be changed, the deserialize
	 * operation can only perform once for the given json string.
	 */
	template <class T, class = void /* for enable_if */>
	class JsonDeserializer {
	public:
		/** Deserialize json to model */
		static std::optional<DeserializeException> deserialize(
			T& model, SharedString& str) {
			JsonDocument document = sajson::parse_single_allocation(
				sajson::mutable_string_view(str.size(), str.data()));
			if (CPV_UNLIKELY(!document.is_valid())) {
				return DeserializeException(CPV_CODEINFO,
					document.get_error_message_as_cstring());
			}
			// sajson will ensure root type is either object or array, no need to check it
			JsonValue root(document.get_root(), str);
			if (CPV_UNLIKELY(!JsonValueConverter<T>::convert(model, root))) {
				return DeserializeException(CPV_CODEINFO, "convert failed");
			}
			return std::nullopt;
		}
	};

	/** Specialize for std::optional<JsonDocument> */
	template <>
	class JsonDeserializer<std::optional<JsonDocument>> {
	public:
		/** Deserialize json to model */
		static std::optional<DeserializeException> deserialize(
			std::optional<JsonDocument>& model, SharedString& str) {
			JsonDocument document = sajson::parse_single_allocation(
				sajson::mutable_string_view(str.size(), str.data()));
			if (CPV_UNLIKELY(!document.is_valid())) {
				return DeserializeException(CPV_CODEINFO,
					document.get_error_message_as_cstring());
			}
			model.emplace(std::move(document));
			return std::nullopt;
		}
	};

	// We can't provide a specialize for JsonValue because
	// the storage of ast tree is inside JsonDocument,
	// we need to hold the JsonDocument until all JsonValue destroyed.

	/** Convenient static function for JsonDeserializer */
	template <class T>
	static inline std::optional<DeserializeException> deserializeJson(
		T& model, SharedString& str) {
		return JsonDeserializer<T>::deserialize(model, str);
	}
}

// use sajson namespace for ADL
namespace sajson {
	using namespace cpv;

	/** Convenient operator overload for JsonValueConverter */
	template <class T>
	static inline void operator<<(T& model, const JsonValue& value) {
		JsonValueConverter<T>::convert(model, value);
	}
}

