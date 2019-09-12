#include <time.h>
#include <cstdlib>
#include <array>
#include <CPVFramework/Utility/DateUtils.hpp>
#include <CPVFramework/Exceptions/LengthException.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestDateUtils, formatTimeForHttpHeader) {
	// time_t always in utc
	{
		std::array<char, 30> buf;
		ASSERT_TRUE(cpv::formatTimeForHttpHeader(0, buf.data(), buf.size()));
		ASSERT_EQ(std::string(buf.data()), "Thu, 01 Jan 1970 00:00:00 GMT");
		ASSERT_TRUE(cpv::formatTimeForHttpHeader(102400, buf.data(), buf.size()));
		ASSERT_EQ(std::string(buf.data()), "Fri, 02 Jan 1970 04:26:40 GMT");
	}
	{
		std::array<char, 29> buf;
		ASSERT_FALSE(cpv::formatTimeForHttpHeader(0, buf.data(), buf.size()));
	}
}

TEST(TestDateUtils, formatTimeForHttpHeader_threadLocal) {
	for (std::size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(cpv::formatTimeForHttpHeader(0), "Thu, 01 Jan 1970 00:00:00 GMT");
		ASSERT_EQ(cpv::formatTimeForHttpHeader(102400), "Fri, 02 Jan 1970 04:26:40 GMT");
	}
}

TEST(TestDateUtils, formatNowForHttpHeader) {
	for (std::size_t i = 0; i < 3; ++i) {
		ASSERT_EQ(cpv::formatNowForHttpHeader().size(), 29U);
	}
}

