#pragma once
#include <type_traits>
#include <nlohmann/json_fwd.hpp>

/**
 * This is for header files, doesn't include detail of json class.
 * There two way to specific how to serialize and deserialize a value:
 * - specialize JsonSerializer<T>
 * - provide toJson and fromJson function inside the class
 */

namespace cpv {
	using Json = nlohmann::json;

	template <class T, class=void>
	struct JsonSerializer {
		static void toJson(Json&, const T&) = delete;
		static void fromJson(const Json&, T&) = delete;
	};

	template <class T>
	struct JsonSerializer<T, std::enable_if_t<
		sizeof(&T::toJson) &&
		sizeof(&T::fromJson)>> {
		static void toJson(Json& json, const T& value) {
			value.toJson(json);
		}
		static void fromJson(const Json& json, T& value) {
			value.fromJson(json);
		}
	};
}

namespace nlohmann {
	template <class T>
	struct adl_serializer<T, std::enable_if_t<
		sizeof(&cpv::JsonSerializer<T>::toJson) &&
		sizeof(&cpv::JsonSerializer<T>::fromJson)>> {
		static void to_json(json& json, const T& value) {
			cpv::JsonSerializer<T>::toJson(json, value);
		}
		static void from_json(const json& json, T& value) {
			cpv::JsonSerializer<T>::fromJson(json, value);
		}
	};
}

