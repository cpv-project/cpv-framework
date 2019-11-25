#pragma once
#include <cstdint>
#include <utility>
#include <chrono>
#include "./SharedString.hpp"

namespace cpv {
	using UUIDDataType = std::pair<std::uint64_t, std::uint64_t>;
	static const std::uint8_t EmptyUUIDVersion = 0;
	static const std::uint8_t TimeUUIDVersion = 1;
	static const std::uint8_t RandomUUIDVersion = 4;

	/** Set the uuid by it's string representation */
	UUIDDataType strToUUID(std::string_view str);

	/** Get the string representation of uuid */
	SharedString uuidToStr(const UUIDDataType& uuid);

	/** Make a empty uuid */
	UUIDDataType makeEmptyUUID();

	/** Make a version 4 (random) uuid */
	UUIDDataType makeRandomUUID();

	/** Make a version 1 (date-time and MAC address) uuid */
	UUIDDataType makeTimeUUID();

	/** Make a minimum version 1 uuid of the specific time point, useful for search query */
	UUIDDataType makeMinTimeUUID(const std::chrono::system_clock::time_point& timePoint);

	/** Make a maximum version 1 uuid of the specific time point, useful for search query */
	UUIDDataType makeMaxTimeUUID(const std::chrono::system_clock::time_point& timePoint);

	/** Extrat version from uuid */
	std::uint8_t getVersionFromUUID(const UUIDDataType& uuid);

	/** Extract time stamp from verion 1 uuid */
	std::chrono::system_clock::time_point getTimeFromUUID(const UUIDDataType& uuid);
}

