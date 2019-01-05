#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <type_traits>
#include <string_view>

namespace cpv {
	/** Bitwise or operation, for enum types under cpv namespace and called by ADL */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	T operator|(T a, T b) {
		using t = std::underlying_type_t<T>;
		return static_cast<T>(static_cast<t>(a) | static_cast<t>(b));
	}

	/** Bitwise or operation */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	T& operator|=(T& a, T b) {
		using t = std::underlying_type_t<T>;
		a = static_cast<T>(static_cast<t>(a) | static_cast<t>(b));
		return a;
	}

	/** Bitwise and operation */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	T operator&(T a, T b) {
		using t = std::underlying_type_t<T>;
		return static_cast<T>(static_cast<t>(a) & static_cast<t>(b));
	}

	/** Bitwise and operation */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	T& operator&=(T& a, T b) {
		using t = std::underlying_type_t<T>;
		a = static_cast<T>(static_cast<t>(a) & static_cast<t>(b));
		return a;
	}

	/** Bitwise not operation */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	T operator~(T a) {
		using t = std::underlying_type_t<T>;
		return static_cast<T>(~static_cast<t>(a));
	}

	/**
	 * Test is enum value not equal to 0
	 * As operation bool can't be static so provide a standalone function.
	 */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	bool enumTrue(T a) {
		using t = std::underlying_type_t<T>;
		return static_cast<t>(a) != 0;
	}

	/** Test is enum value equal to 0 */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	bool enumFalse(T a) {
		using t = std::underlying_type_t<T>;
		return static_cast<t>(a) == 0;
	}

	/** Get the integer value of enum */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	std::underlying_type_t<T> enumValue(T a) {
		using t = std::underlying_type_t<T>;
		return static_cast<t>(a);
	}

	/** Specialize this class to provide enum descriptions */
	template <class T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
	struct EnumDescriptions {
		static const std::vector<std::pair<T, const char*>>& get() = delete;
	};

	/** Write text description of enum to stream */
	template <class T, std::enable_if_t<sizeof(EnumDescriptions<T>::get()), int> = 0>
	std::ostream& operator<<(std::ostream& stream, T value) {
		auto& descriptions = EnumDescriptions<T>::get();
		// find the value exactly matched
		for (const auto& pair : descriptions) {
			if (value == pair.first) {
				stream << pair.second;
				return stream;
			}
		}
		// find the bits combination of the value
		bool isFirst = true;
		for (const auto& pair : descriptions) {
			if (enumValue(pair.first) == 0) {
				continue;
			}
			if ((value & pair.first) == pair.first) {
				if (isFirst) {
					isFirst = false;
				} else {
					stream << "|";
				}
				stream << pair.second;
			}
		}
		return stream;
	}

	/** Get the enum value from it's description string */
	template <class T, std::enable_if_t<sizeof(EnumDescriptions<T>::get()), int> = 0>
	T enumFromString(const std::string_view& str, T defaultValue = {}) {
		auto& descriptions = EnumDescriptions<T>::get();
		for (const auto& pair : descriptions) {
			if (str == pair.second) {
				return pair.first;
			}
		}
		return defaultValue;
	}
}
