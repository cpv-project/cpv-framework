#include <CPVFramework/Utility/StringUtils.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestStringUtils, split) {
	std::vector<std::string> results;
	std::size_t countRecord = 0;
	std::string str("aaa \tb  c\nd eee");
	cpv::splitString(str,
		[&results, &countRecord, &str](auto startIndex, auto endIndex, auto count) {
			ASSERT_EQ(countRecord, count);
			++countRecord;
			results.emplace_back(str, startIndex, endIndex - startIndex);
		});
	ASSERT_EQ(results.size(), 5U);
	ASSERT_EQ(results.at(0), "aaa");
	ASSERT_EQ(results.at(1), "b");
	ASSERT_EQ(results.at(2), "c");
	ASSERT_EQ(results.at(3), "d");
	ASSERT_EQ(results.at(4), "eee");
}

TEST(TestStringUtils, join) {
	std::string a = cpv::joinString(" ", "a", 123, "asd");
	ASSERT_EQ(a, "a 123 asd");
	std::string b = cpv::joinString("-", "b");
	ASSERT_EQ(b, "b");
	std::string c = cpv::joinString("", 1, 2, 888);
	ASSERT_EQ(c, "12888");
}

TEST(TestStringUtils, dumpIntToHex) {
	{
		std::string str;
		cpv::dumpIntToHex<std::uint8_t>(0x7f, str);
		ASSERT_EQ(str, "7F");
	}
	{
		std::string str;
		cpv::dumpIntToHex<std::int32_t>(0x12345678, str);
		ASSERT_EQ(str, "12345678");
	}
	{
		std::string str;
		cpv::dumpIntToHex<std::int32_t>(-0x12345678, str);
		ASSERT_EQ(str, "EDCBA988");
	}
}

TEST(TestStringUtils, dumpIntToDec) {
	{
		std::string str;
		cpv::dumpIntToDec<std::uint8_t>(123, str);
		ASSERT_EQ(str, "123");
	}
	{
		std::string str;
		cpv::dumpIntToDec<std::int8_t>(-128, str);
		ASSERT_EQ(str, "-128");
	}
	{
		std::string str;
		cpv::dumpIntToDec<std::uint8_t>(0, str);
		ASSERT_EQ(str, "0");
	}
}

TEST(TestStringUtils, dumpBytesToHex) {
	{
		std::string str;
		cpv::dumpBytesToHex("abc", 3, str);
		ASSERT_EQ(str, "616263");
	}
	{
		std::string str;
		cpv::dumpBytesToHex("", 0, str);
		ASSERT_EQ(str, "");
	}
}

TEST(TestStringUtils, loadIntFromHex) {
	{
		std::uint8_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromHex("7F", value));
		ASSERT_EQ(value, 0x7f);
	}
	{
		std::uint8_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromHex("7f", value));
		ASSERT_EQ(value, 0x7f);
	}
	{
		std::int32_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromHex("12345678", value));
		ASSERT_EQ(value, 0x12345678);
	}
	{
		std::int32_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromHex("EDCBA988", value));
		ASSERT_EQ(value, -0x12345678);
	}
	{
		std::int32_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromHex("EDCBa988", value));
		ASSERT_EQ(value, -0x12345678);
	}
	{
		std::int32_t value = 0;
		ASSERT_FALSE(cpv::loadIntFromHex("EDCBag88", value));
	}
	{
		std::int32_t value = 0;
		ASSERT_FALSE(cpv::loadIntFromHex("EDCBa98", value));
	}
}

TEST(TestStringUtils, loadIntFromDec) {
	{
		std::int32_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromDec("123456789", 9, value));
		ASSERT_EQ(value, 123456789);
	}
	{
		std::int32_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromDec("987654321", 9, value));
		ASSERT_EQ(value, 987654321);
	}
	{
		std::int32_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromDec("-579", 4, value));
		ASSERT_EQ(value, -579);
	}
	{
		std::int32_t value = 0;
		ASSERT_FALSE(cpv::loadIntFromDec("123a", 4, value));
	}
	{
		std::int32_t value = 0;
		ASSERT_FALSE(cpv::loadIntFromDec("-123a", 5, value));
	}
	{
		std::int32_t value = 123;
		ASSERT_TRUE(cpv::loadIntFromDec("", 0, value));
		ASSERT_EQ(value, 0);
	}
	{
		std::uint32_t value = 0;
		ASSERT_TRUE(cpv::loadIntFromDec("918273645", 9, value));
		ASSERT_EQ(value, 918273645U);
	}
	{
		std::uint32_t value = 0;
		ASSERT_FALSE(cpv::loadIntFromDec("-918273645", 10, value));
	}
}

TEST(TestStringUtils, loadBytesFromHex) {
	{
		std::string str;
		ASSERT_TRUE(cpv::loadBytesFromHex("616263", 6, str));
		ASSERT_EQ(str, "abc");
	}
	{
		std::string str;
		ASSERT_TRUE(cpv::loadBytesFromHex("aa7F001f", 8, str));
		ASSERT_EQ(str, std::string("\xaa\x7f\x00\x1f", 4));
	}
	{
		std::string str;
		ASSERT_FALSE(cpv::loadBytesFromHex("aa7F001g", 8, str));
	}
	{
		std::string str;
		ASSERT_FALSE(cpv::loadBytesFromHex("aa7F00", 8, str));
	}
}

TEST(TestStringUtils, caseInsensitiveEquals) {
	ASSERT_TRUE(cpv::caseInsensitiveEquals(std::string("abc"), std::string("Abc")));
	ASSERT_TRUE(cpv::caseInsensitiveEquals(std::string("ABC012"), std::string("abc012")));
	ASSERT_FALSE(cpv::caseInsensitiveEquals(std::string("ABC"), std::string("ab0")));
	ASSERT_FALSE(cpv::caseInsensitiveEquals(std::string("ABC"), std::string("abd")));
}

