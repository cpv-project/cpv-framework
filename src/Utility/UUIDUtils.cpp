#include <random>
#include <seastar/core/byteorder.hh>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Exceptions/UUIDConflictException.hpp>
#include <CPVFramework/Utility/SharedStringBuilder.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Utility/UUIDUtils.hpp>

namespace cpv {
	namespace {
		// NOTICE: random_device may not work on windows,
		// but this library also does not work on windows
		thread_local std::random_device URandom;
		static_assert(sizeof(URandom()) >= sizeof(std::uint32_t),
			"random_device should return a atleast 32 bit integer");
		static const std::uint32_t MaxClockId = 0b11'1111'1111'1111; // 16383
		static const std::uint64_t EpochSinceUUID1TimestampBase = 122'192'928'000'000'000; // 1582-10-15
		
		/** Convert a time point to time stamp of uuid 1 */
		std::uint64_t toTimeUUIDTimestamp(const std::chrono::system_clock::time_point& timePoint) {
			// the number of 100 nanoseconds intervals since 1582-10-15
			std::chrono::system_clock::duration sinceEpoch = timePoint.time_since_epoch();
			std::uint64_t timeStamp = (
				(std::chrono::duration_cast<
					std::chrono::milliseconds>(sinceEpoch).count() * 10000ULL) +
				((std::chrono::duration_cast<
					std::chrono::nanoseconds>(sinceEpoch).count() / 100ULL) % 10000) +
				EpochSinceUUID1TimestampBase);
			return timeStamp;
		}

		/** Convert a time stamp of uuid 1 to time point */
		std::chrono::system_clock::time_point fromTimeUUIDTimestamp(std::uint64_t timeStamp) {
			auto result = std::chrono::system_clock::from_time_t(0);
			result += std::chrono::milliseconds(
				(timeStamp - EpochSinceUUID1TimestampBase) / 10000);
			result += std::chrono::nanoseconds((timeStamp % 10000) * 100);
			return result;
		}
	}

	/** Set the uuid by it's string representation */
	UUIDDataType strToUUID(std::string_view str) {
		// example: 00112233-4455-6677-8899-aabbccddeeff
		if (CPV_UNLIKELY(str.size() != 36)) {
			throw FormatException(CPV_CODEINFO,
				"invalid uuid string: size should be 36, str is", str);
		}
		std::uint32_t a = 0;
		std::uint16_t b = 0;
		std::uint16_t c = 0;
		std::uint16_t d = 0;
		std::uint16_t e = 0;
		std::uint32_t f = 0;
		if (CPV_UNLIKELY(
			!loadIntFromHex(str.data(), a) ||
			!loadIntFromHex(str.data() + 9, b) ||
			!loadIntFromHex(str.data() + 14, c) ||
			!loadIntFromHex(str.data() + 19, d) ||
			!loadIntFromHex(str.data() + 24, e) ||
			!loadIntFromHex(str.data() + 28, f))) {
			throw FormatException(CPV_CODEINFO,
				"invalid uuid string: contains non-hex character, str is", str);
		}
		std::uint64_t highBits = (
			static_cast<std::uint64_t>(a) << 32) | (static_cast<std::uint64_t>(b) << 16) | c;
		std::uint64_t lowBits = (
			static_cast<std::uint64_t>(d) << 48) | (static_cast<std::uint64_t>(e) << 32) | f;
		return UUIDDataType(highBits, lowBits);
	}

	/** Get the string representation of uuid */
	SharedString uuidToStr(const UUIDDataType& uuid) {
		// example: 00112233-4455-6677-8899-aabbccddeeff
		SharedStringBuilder builder(36);
		dumpIntToHex(static_cast<std::uint32_t>(uuid.first >> 32), builder);
		builder.append(1, '-');
		dumpIntToHex(static_cast<std::uint16_t>((uuid.first >> 16) & 0xffff), builder);
		builder.append(1, '-');
		dumpIntToHex(static_cast<std::uint16_t>(uuid.first & 0xffff), builder);
		builder.append(1, '-');
		dumpIntToHex(static_cast<std::uint16_t>((uuid.second >> 48) & 0xffff), builder);
		builder.append(1, '-');
		dumpIntToHex(static_cast<std::uint16_t>((uuid.second >> 32) & 0xffff), builder);
		dumpIntToHex(static_cast<std::uint32_t>(uuid.second & 0xffffffff), builder);
		return builder.build();
	}

	/** Make a empty uuid */
	UUIDDataType makeEmptyUUID() {
		return UUIDDataType(0, 0);
	}

	/** Make a version 4 (random) uuid */
	UUIDDataType makeRandomUUID() {
		UUIDDataType uuid;
		uuid.first = (static_cast<std::uint64_t>(URandom()) << 32) | URandom();
		uuid.second = (static_cast<std::uint64_t>(URandom()) << 32) | URandom();
		uuid.first = (uuid.first & 0xffff'ffff'ffff'0fff) | 0x4000; // version 4
		uuid.second = (uuid.second & 0x3fff'ffff'ffff'ffff) | 0x8000'0000'0000'0000; // variant 1
		return uuid;
	}

	/** Make a version 1 (date-time and MAC address) uuid */
	UUIDDataType makeTimeUUID(
		std::uint64_t timeStamp, std::uint32_t clockId, std::uint64_t nodeId) {
		// check: https://www.famkruithof.net/guid-uuid-timebased.html
		// example: 58e0a7d7-eebc-11d8-9669-0800200c9a66
		// - timestamp: 1db'eebc'58e0a7d7
		// - clock id: 1669
		// - node id: 0800200c9a66
		UUIDDataType uuid;
		uuid.first = (
			((timeStamp & 0x0000'0000'ffff'ffffULL) << 32) |
			((timeStamp & 0x0000'ffff'0000'0000ULL) >> 16) |
			0x0000'0000'0000'1000ULL | // version 1
			((timeStamp & 0x0fff'0000'0000'0000ULL) >> 48));
		uuid.second = (
			0x8000'0000'0000'0000ULL | //variant 1
			((clockId & 0x3fffULL) << 48) |
			(nodeId & 0x0000'ffff'ffff'ffffULL));
		return uuid;
	}

	/** Make a version 1 (date-time and MAC address) uuid */
	UUIDDataType makeTimeUUID() {
		thread_local static std::uint64_t staticNodeId = 0;
		thread_local static std::uint64_t staticLastTimestamp = 0;
		thread_local static std::uint32_t staticClockId = 0;
		if (CPV_UNLIKELY(staticNodeId == 0)) {
			// use random value as mac address because each core won't share clock id
			staticNodeId = (static_cast<std::uint64_t>(URandom()) << 32) | URandom();
		}
		std::uint64_t timeStamp = toTimeUUIDTimestamp(std::chrono::system_clock::now());
		if (timeStamp != staticLastTimestamp) {
			staticLastTimestamp = timeStamp;
			staticClockId = 0;
		} else if (CPV_UNLIKELY(staticClockId > MaxClockId)) {
			throw UUIDConflictException(CPV_CODEINFO,
				"generate time uuid failed, clock id is exhausted in this 100 nanoseconds");
		}
		std::uint32_t clockId = staticClockId++;
		return makeTimeUUID(timeStamp, clockId, staticNodeId);
	}

	/** Make a minimum version 1 uuid of the specific time point, useful for search query */
	UUIDDataType makeMinTimeUUID(const std::chrono::system_clock::time_point& timePoint) {
		std::uint64_t timeStamp = toTimeUUIDTimestamp(timePoint);
		return makeTimeUUID(timeStamp, 0, 0);
	}

	/** Make a maximum version 1 uuid of the specific time point, useful for search query */
	UUIDDataType makeMaxTimeUUID(const std::chrono::system_clock::time_point& timePoint) {
		std::uint64_t timeStamp = toTimeUUIDTimestamp(timePoint);
		return makeTimeUUID(timeStamp, MaxClockId, 0xffff'ffff'ffff'ffffULL);
	}

	/** Extrat version from uuid */
	std::uint8_t getVersionFromUUID(const UUIDDataType& uuid) {
		return (uuid.first & 0x0000'0000'0000'f000ULL) >> 12;
	}

	/** Extract time stamp from verion 1 uuid */
	std::chrono::system_clock::time_point getTimeFromUUID(const UUIDDataType& uuid) {
		if (CPV_UNLIKELY(getVersionFromUUID(uuid) != TimeUUIDVersion)) {
			// version 2 is unsupported yet
			throw FormatException(CPV_CODEINFO,
				"get time from uuid failed: version should be 1, uuid is", uuidToStr(uuid));
		}
		std::uint64_t timeStamp = (
			((uuid.first & 0xffff'ffff'0000'0000ULL) >> 32) |
			((uuid.first & 0x0000'0000'ffff'0000ULL) << 16) |
			((uuid.first & 0x0000'0000'0000'0fffULL) << 48));
		return fromTimeUUIDTimestamp(timeStamp);
	}
}

