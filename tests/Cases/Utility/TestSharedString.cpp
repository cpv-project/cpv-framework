#include <map>
#include <unordered_map>
#include <CPVFramework/Utility/SharedString.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestSharedString, construct) {
	ASSERT_EQ(cpv::SharedString(), "");
	ASSERT_EQ(cpv::SharedString("test static"), "test static");
	ASSERT_EQ(cpv::SharedString(std::string("test copy")), "test copy");
	ASSERT_EQ(cpv::SharedString::fromStatic("test static"), "test static");
	ASSERT_EQ(cpv::SharedString::fromInt(123), "123");
	ASSERT_EQ(cpv::SharedString::fromInt(12345678), "12345678");
	ASSERT_EQ(cpv::SharedString::fromInt(123U), "123");
	ASSERT_EQ(cpv::SharedString::fromInt(12345678U), "12345678");
	ASSERT_EQ(cpv::SharedString::fromDouble(1.0), "1");
	ASSERT_EQ(cpv::SharedString::fromDouble(0.1), "0.1");
	ASSERT_EQ(cpv::SharedString::fromDouble(static_cast<long double>(1.0)), "1");
	ASSERT_EQ(cpv::SharedString::fromDouble(static_cast<long double>(0.1)), "0.1");
	{
		cpv::SharedString str = seastar::temporary_buffer<char>("abc", 3);
		ASSERT_EQ(str, "abc");
	}
}

TEST(TestSharedString, data) {
	cpv::SharedString str("abc");
	ASSERT_EQ(std::string_view(str.data(), str.size()), "abc");
	ASSERT_EQ(std::string_view(
		static_cast<const cpv::SharedString&>(str).data(), str.size()), "abc");
}

TEST(TestSharedString, view) {
	cpv::SharedString str("abc");
	ASSERT_EQ(str.view(), "abc");
	std::string_view view = str;
	ASSERT_EQ(view, "abc");
}

TEST(TestSharedString, buffer) {
	cpv::SharedString str(std::string("abc"));
	seastar::temporary_buffer<char> bufShared = str.buffer();
	ASSERT_EQ(str, "abc");
	ASSERT_EQ(std::string_view(bufShared.get(), bufShared.size()), "abc");
	seastar::temporary_buffer<char> bufMoved = std::move(str).buffer();
	ASSERT_EQ(str, "");
	ASSERT_EQ(std::string_view(bufMoved.get(), bufMoved.size()), "abc");
}

TEST(TestSharedString, share) {
	cpv::SharedString str("abc def");
	cpv::SharedString strShared = str.share();
	cpv::SharedString strAbc = str.share(0, 3);
	cpv::SharedString strDef = str.share(4);
	cpv::SharedString strCde = str.share(str.view().substr(2, 4));
	ASSERT_EQ(str, "abc def");
	ASSERT_EQ(strShared, "abc def");
	ASSERT_EQ(strAbc, "abc");
	ASSERT_EQ(strDef, "def");
	ASSERT_EQ(strCde, "c de");
}

TEST(TestSharedString, compare) {
	{
		ASSERT_TRUE(cpv::SharedString("abc") == cpv::SharedString("abc"));
		ASSERT_FALSE(cpv::SharedString("abc") == cpv::SharedString("def"));
		ASSERT_TRUE(cpv::SharedString("abc") == std::string_view("abc"));
		ASSERT_FALSE(cpv::SharedString("abc") == std::string_view("def"));
		ASSERT_TRUE(cpv::SharedString("abc") == "abc");
		ASSERT_FALSE(cpv::SharedString("abc") == "def");
	}
	{
		ASSERT_TRUE(cpv::SharedString("abc") != cpv::SharedString("def"));
		ASSERT_FALSE(cpv::SharedString("abc") != cpv::SharedString("abc"));
		ASSERT_TRUE(cpv::SharedString("abc") != std::string_view("def"));
		ASSERT_FALSE(cpv::SharedString("abc") != std::string_view("abc"));
		ASSERT_TRUE(cpv::SharedString("abc") != "def");
		ASSERT_FALSE(cpv::SharedString("abc") != "abc");
	}
	{
		ASSERT_TRUE(cpv::SharedString("abc100") < cpv::SharedString("abc101"));
		ASSERT_FALSE(cpv::SharedString("abc100") < cpv::SharedString("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc101") < cpv::SharedString("abc100"));
		ASSERT_TRUE(cpv::SharedString("abc100") < std::string_view("abc101"));
		ASSERT_FALSE(cpv::SharedString("abc100") < std::string_view("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc101") < std::string_view("abc100"));
		ASSERT_TRUE(cpv::SharedString("abc100") < "abc101");
		ASSERT_FALSE(cpv::SharedString("abc100") < "abc100");
		ASSERT_FALSE(cpv::SharedString("abc101") < "abc100");
	}
	{
		ASSERT_TRUE(cpv::SharedString("abc100") <= cpv::SharedString("abc101"));
		ASSERT_TRUE(cpv::SharedString("abc100") <= cpv::SharedString("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc101") <= cpv::SharedString("abc100"));
		ASSERT_TRUE(cpv::SharedString("abc100") <= std::string_view("abc101"));
		ASSERT_TRUE(cpv::SharedString("abc100") <= std::string_view("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc101") <= std::string_view("abc100"));
		ASSERT_TRUE(cpv::SharedString("abc100") <= "abc101");
		ASSERT_TRUE(cpv::SharedString("abc100") <= "abc100");
		ASSERT_FALSE(cpv::SharedString("abc101") <= "abc100");
	}
	{
		ASSERT_TRUE(cpv::SharedString("abc101") > cpv::SharedString("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc100") > cpv::SharedString("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc100") > cpv::SharedString("abc101"));
		ASSERT_TRUE(cpv::SharedString("abc101") > std::string_view("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc100") > std::string_view("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc100") > std::string_view("abc101"));
		ASSERT_TRUE(cpv::SharedString("abc101") > "abc100");
		ASSERT_FALSE(cpv::SharedString("abc100") > "abc100");
		ASSERT_FALSE(cpv::SharedString("abc100") > "abc101");
	}
	{
		ASSERT_TRUE(cpv::SharedString("abc101") >= cpv::SharedString("abc100"));
		ASSERT_TRUE(cpv::SharedString("abc100") >= cpv::SharedString("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc100") >= cpv::SharedString("abc101"));
		ASSERT_TRUE(cpv::SharedString("abc101") >= std::string_view("abc100"));
		ASSERT_TRUE(cpv::SharedString("abc100") >= std::string_view("abc100"));
		ASSERT_FALSE(cpv::SharedString("abc100") >= std::string_view("abc101"));
		ASSERT_TRUE(cpv::SharedString("abc101") >= "abc100");
		ASSERT_TRUE(cpv::SharedString("abc100") >= "abc100");
		ASSERT_FALSE(cpv::SharedString("abc100") >= "abc101");
	}
}

TEST(TestSharedString, mapKey) {
	auto test = [](auto& map) {
		map.emplace("a", 1);
		map.emplace("b", 2);
		map.emplace("aa", 11);
		ASSERT_EQ(map.size(), 3U);
		ASSERT_EQ(map.count("a"), 1U);
		ASSERT_EQ(map.at("a"), 1);
		ASSERT_EQ(map.count("b"), 1U);
		ASSERT_EQ(map.at("b"), 2);
		ASSERT_EQ(map.count("c"), 0U);
		ASSERT_EQ(map.count("aa"), 1U);
		ASSERT_EQ(map.at("aa"), 11);
	};
	{
		std::map<cpv::SharedString, int> map;
		test(map);
	}
	{
		std::unordered_map<cpv::SharedString, int> map;
		test(map);
	}
}

