#include <CPVFramework/Utility/HttpUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(HttpUtils, urlEncode) {
	{
		auto result = cpv::urlEncode("一abc二def三 ~-_\r\n");
		ASSERT_EQ(result, "%E4%B8%80abc%E4%BA%8Cdef%E4%B8%89%20~-_%0D%0A");
	}
	{
		auto result = cpv::urlEncode("qwert.12345");
		ASSERT_EQ(result, "qwert.12345");
	}
}

TEST(HttpUtils, urlDecode) {
	{
		auto result = cpv::urlDecode("%E4%B8%80abc%E4%BA%8Cdef%E4%B8%89%20~-_%0D%0A");
		ASSERT_EQ(result, "一abc二def三 ~-_\r\n");
	}
	{
		auto result = cpv::urlDecode("%E4%B8%80abc%E4%BA%8Cdef%E4%B8%89+~-_%0D%0A");
		ASSERT_EQ(result, "一abc二def三 ~-_\r\n");
	}
	{
		auto result = cpv::urlDecode("qwert.12345");
		ASSERT_EQ(result, "qwert.12345");
	}
	{
		auto result = cpv::urlDecode("");
		ASSERT_EQ(result, "");
	}
	{
		auto result = cpv::urlDecode("asd%E");
		ASSERT_EQ(result, "asd");
	}
	{
		auto result = cpv::urlDecode("321%");
		ASSERT_EQ(result, "321");
	}
	{
		auto result = cpv::urlDecode("%PP");
		ASSERT_EQ(result, cpv::SharedString("\x00"));
	}
}

TEST(HttpUtils, htmlEncode) {
	{
		auto result = cpv::htmlEncode("&一<abc>二'def'三\"& ~-_\"\r\n&");
		ASSERT_EQ(result, "&amp;一&lt;abc&gt;二&#x27;def&#x27;三&quot;&amp; ~-_&quot;\r\n&amp;");
	}
	{
		auto result = cpv::htmlEncode("qwert.12345");
		ASSERT_EQ(result, "qwert.12345");
	}
}

// Notice: cases with incorrect format are for ASAN checking, their results are not guaranteed
TEST(HttpUtils, htmlDecode) {
	{
		auto result = cpv::htmlDecode("&amp;一&lt;abc&gt;二&#x27;def&#x27;三&quot;&amp; ~-_&quot;\r\n&amp;");
		ASSERT_EQ(result, "&一<abc>二'def'三\"& ~-_\"\r\n&");
	}
	{
		auto result = cpv::htmlDecode("qwert.12345");
		ASSERT_EQ(result, "qwert.12345");
	}
	{
		auto result = cpv::htmlDecode("");
		ASSERT_EQ(result, "");
	}
	{
		auto result = cpv::htmlDecode("&quot;&#x4e00;+&#x4e8c;=&#X4e09;&QUOT;");
		ASSERT_EQ(result, "\"一+二=三\"");
	}
	{
		auto result = cpv::htmlDecode("&#x1D56B;&#x7e;&#120171;");
		ASSERT_EQ(result, "𝕫~𝕫");
	}
	{
		auto result = cpv::htmlDecode("ab&#xpp;c");
		ASSERT_EQ(result, "abc");
	}
	{
		auto result = cpv::htmlDecode("ab&#x;c");
		ASSERT_EQ(result, std::string_view("ab\x00""c", 4));
	}
	{
		auto result = cpv::htmlDecode("ab&#;c");
		ASSERT_EQ(result, "abc");
	}
	{
		auto result = cpv::htmlDecode("ab&#x200");
		ASSERT_EQ(result, "ab ");
	}
	{
		auto result = cpv::htmlDecode("ab&#x1");
		ASSERT_EQ(result, std::string_view("ab\x00", 3));
	}
	{
		auto result = cpv::htmlDecode("ab&#x");
		ASSERT_EQ(result, "ab");
	}
	{
		auto result = cpv::htmlDecode("ab&#");
		ASSERT_EQ(result, "ab");
	}
	{
		auto result = cpv::htmlDecode("ab&;");
		ASSERT_EQ(result, "ab");
	}
	{
		auto result = cpv::htmlDecode("ab&#");
		ASSERT_EQ(result, "ab");
	}
	{
		auto result = cpv::htmlDecode("ab&");
		ASSERT_EQ(result, "ab");
	}
}

TEST(HttpUtils, getMimeType) {
	ASSERT_EQ(cpv::getMimeType("zip"), "application/zip");
	ASSERT_EQ(cpv::getMimeType("./path/some.test.json"), "application/json");
	ASSERT_EQ(cpv::getMimeType("filename.unknown"), "application/octet-stream");
}

