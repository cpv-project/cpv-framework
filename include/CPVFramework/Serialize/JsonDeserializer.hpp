#pragma once
#include <memory>
#include <optional>
#include <string_view>
#include <seastar/core/shared_ptr.hh>
#include <seastar/core/temporary_buffer.hh>
#include "../Allocators/StackAllocator.hpp"
#include "../Exceptions/DeserializeException.hpp"
#include "../Utility/Reusable.hpp"
#include "./JsonDeserializer.sajson.hpp"

namespace cpv {
	// alias of sajson types
	using JsonType = sajson::type;
	using JsonValue = sajson::value;
	using JsonDocument = sajson::document;

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

	/** Specialize for std::string_view */
	template <>
	struct JsonValueConverter<std::string_view> {
		/** Convert json value to std::string_view if type matched */
		static bool convert(std::string_view& target, const JsonValue& value) {
			if (CPV_LIKELY(value.get_type() == JsonType::TYPE_STRING)) {
				target = std::string_view(value.as_cstring(), value.get_string_length());
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

	/** Specialize for std::vector */
	template <class T, class Allocator>
	struct JsonValueConverter<std::vector<T, Allocator>> {
		/** Convert json value to std::vector if type matched */
		static bool convert(std::vector<T, Allocator>& models, const JsonValue& value) {
			if (CPV_LIKELY(value.get_type() == JsonType::TYPE_ARRAY)) {
				std::size_t length = value.get_length();
				models.reserve(models.size() + length);
				bool result = true;
				for (std::size_t i = 0; i < length; ++i) {
					result = JsonValueConverter<T>::convert(
						models.emplace_back(), value.get_array_element(i)) && result;
				}
				return result;
			}
			return false;
		}
	};

	/** Specialize for StackAllocatedVector */
	template <class T, std::size_t InitialSize, class UpstreamAllocator>
	struct JsonValueConverter<StackAllocatedVector<T, InitialSize, UpstreamAllocator>> :
		public JsonValueConverter<std::vector<T, typename
			StackAllocatedVector<T, InitialSize, UpstreamAllocator>::allocator_type>> { };

	/** Specialize for std::optional */
	template <class T>
	struct JsonValueConverter<std::optional<T>> {
		/** Convert json value to std::optional if type matched */
		static bool convert(std::optional<T>& target, const JsonValue& value) {
			if (value.get_type() == JsonType::TYPE_NULL) {
				target.reset();
				return true;
			} else {
				if (!target.has_value()) {
					target.emplace();
				}
				return JsonValueConverter<T>::convert(*target, value);
			}
		}
	};

	/** Specialize for std::unique_ptr */
	template <class T>
	struct JsonValueConverter<std::unique_ptr<T>> {
		/** Convert json value to std::unique_ptr if type matched */
		static bool convert(std::unique_ptr<T>& target, const JsonValue& value) {
			if (value.get_type() == JsonType::TYPE_NULL) {
				target.reset();
				return true;
			} else {
				if (target == nullptr) {
					target = std::make_unique<T>();
				}
				return JsonValueConverter<T>::convert(*target, value);
			}
		}
	};

	/** Specialize for seastar::shared_ptr */
	template <class T>
	struct JsonValueConverter<seastar::shared_ptr<T>> {
		/** Convert json value to seastar::shared_ptr if type matched */
		static bool convert(seastar::shared_ptr<T>& target, const JsonValue& value) {
			if (value.get_type() == JsonType::TYPE_NULL) {
				target = nullptr;
				return true;
			} else {
				if (target.get() == nullptr) {
					target = seastar::make_shared<T>();
				}
				return JsonValueConverter<T>::convert(*target, value);
			}
		}
	};

	/** Specialize for Reusable */
	template <class T>
	struct JsonValueConverter<Reusable<T>> {
		/** Convert json value to Reusable if type matched */
		static bool convert(Reusable<T>& target, const JsonValue& value) {
			if (value.get_type() == JsonType::TYPE_NULL) {
				target.reset();
				return true;
			} else {
				if (target == nullptr) {
					target = makeReusable<T>();
				}
				return JsonValueConverter<T>::convert(*target, value);
			}
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
	 * Notice:
	 * The storage of all string values are in the buffer parameter,
	 * if you construct any string view from JsonValue, you must keep
	 * the buffer alive before all related string views destroyed.
	 *
	 * Since it uses sajson, and sajson is a in-situ parser, the buffer
	 * must be mutable so encoded strings can be replaced by decoded strings.
	 * And since the contents of buffer can be changed, the deserialize
	 * operation can only perform once for the given buffer.
	 */
	template <class T, class = void /* for enable_if */>
	class JsonDeserializer {
	public:
		/** Deserialize json to model */
		static std::optional<DeserializeException> deserialize(
			T& model, seastar::temporary_buffer<char>& buffer) {
			JsonDocument document = sajson::parse_single_allocation(
				sajson::mutable_string_view(buffer.size(), buffer.get_write()));
			if (CPV_UNLIKELY(!document.is_valid())) {
				return DeserializeException(CPV_CODEINFO,
					document.get_error_message_as_cstring());
			}
			JsonValue root = document.get_root();
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
			std::optional<JsonDocument>& model, seastar::temporary_buffer<char>& buffer) {
			JsonDocument document = sajson::parse_single_allocation(
				sajson::mutable_string_view(buffer.size(), buffer.get_write()));
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
		T& model, seastar::temporary_buffer<char>& buffer) {
		return JsonDeserializer<T>::deserialize(model, buffer);
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

