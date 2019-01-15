#include <time.h>
#include <cstdlib>
#include <array>
#include <CPVFramework/Utility/DateUtils.hpp>
#include <CPVFramework/Exceptions/LengthException.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(DateUtils, formatTimeForHttpHeader) {
	// time_t always in utc
	{
		std::array<char, 30> buf;
		ASSERT_EQ(cpv::formatTimeForHttpHeader(0, buf.data(), buf.size()), 29U);
		ASSERT_EQ(std::string(buf.data()), "Thu, 01 Jan 1970 00:00:00 GMT");
		ASSERT_EQ(cpv::formatTimeForHttpHeader(102400, buf.data(), buf.size()), 29U);
		ASSERT_EQ(std::string(buf.data()), "Fri, 02 Jan 1970 04:26:40 GMT");
	}
	{
		std::array<char, 29> buf;
		ASSERT_THROWS_CONTAINS(
			cpv::LengthException,
			cpv::formatTimeForHttpHeader(0, buf.data(), buf.size()),
			"buffer size not enough");
	}
}

TEST(DateUtils, formatTimeForHttpHeader_threadLocal) {
	for (std::size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(cpv::formatTimeForHttpHeader(0), "Thu, 01 Jan 1970 00:00:00 GMT");
		ASSERT_EQ(cpv::formatTimeForHttpHeader(102400), "Fri, 02 Jan 1970 04:26:40 GMT");
	}
}

TEST(DateUtils, formatNowForHttpHeader) {
	for (std::size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(cpv::formatNowForHttpHeader().size(), 29U);
	}
}

