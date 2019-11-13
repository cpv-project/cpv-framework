#include <limits>
#include <CPVFramework/Exceptions/OverflowException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestBufferUtils, mergeContent) {
	seastar::temporary_buffer<char> buf;
	std::string_view exists("000");
	// initial merge
	cpv::mergeContent(buf, exists, "11111");
	ASSERT_EQ(exists, "00011111");
	ASSERT_EQ(exists.data(), buf.get());
	ASSERT_LE(exists.size(), buf.size());
	// capacity enough
	cpv::mergeContent(buf, exists, "0000");
	ASSERT_EQ(exists, "000111110000");
	ASSERT_EQ(exists.data(), buf.get());
	ASSERT_LE(exists.size(), buf.size());
	// capacity not enough
	std::string longContent(1024, 'a');
	cpv::mergeContent(buf, exists, longContent);
	ASSERT_EQ(exists, "000111110000" + longContent);
	ASSERT_EQ(exists.data(), buf.get());
	ASSERT_LE(exists.size(), buf.size());
}

TEST(TestBufferUtils, mergeContentError) {
	seastar::temporary_buffer<char> buf;
	std::string_view exists("000");
	ASSERT_THROWS_CONTAINS(
		cpv::OverflowException,
		cpv::mergeContent(buf, exists, std::string_view("", std::numeric_limits<std::size_t>::max() - 1)),
		"size of existsContent + newContent overflowed");
}

TEST(TestBufferUtils, convertIntToBuffer_signed) {
	{
		auto buf = cpv::convertIntToBuffer<std::int64_t>(0);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "0");
	}
	{
		auto buf = cpv::convertIntToBuffer<std::int64_t>(123);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "123");
	}
	{
		auto buf = cpv::convertIntToBuffer<std::int64_t>(0x7fff'ffff'ffff'ffffLL);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "9223372036854775807");
	}
	{
		auto buf = cpv::convertIntToBuffer<std::int64_t>(-0x8000'0000'0000'0000LL);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "-9223372036854775808");
	}
}

TEST(TestBufferUtils, convertIntToBuffer_unsigned) {
	{
		auto buf = cpv::convertIntToBuffer<std::uint64_t>(0);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "0");
	}
	{
		auto buf = cpv::convertIntToBuffer<std::uint64_t>(123);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "123");
	}
	{
		auto buf = cpv::convertIntToBuffer<std::uint64_t>(0xffff'ffff'ffff'ffffLL);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "18446744073709551615");
	}
}

TEST(TestBufferUtils, convertDoubleToBuffer) {
	{
		double value = 0.1;
		auto buf = cpv::convertDoubleToBuffer(value);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "0.1");
	}
	{
		double value = 100;
		auto buf = cpv::convertDoubleToBuffer(value);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "100");
	}
	{
		double value = 0;
		auto buf = cpv::convertDoubleToBuffer(value);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "0");
	}
	{
		long double value = 1.2;
		auto buf = cpv::convertDoubleToBuffer(value);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "1.2");
	}
	{
		long double value = -100.0;
		auto buf = cpv::convertDoubleToBuffer(value);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "-100");
	}
	{
		long double value = 0.0;
		auto buf = cpv::convertDoubleToBuffer(value);
		ASSERT_EQ(std::string_view(buf.get(), buf.size()), "0");
	}
}

