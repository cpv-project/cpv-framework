#include <CPVFramework/Utility/HttpUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestHttpUtils, urlEncode) {
	{
		auto result = cpv::urlEncode("一abc二def三 ~-_\r\n");
		ASSERT_EQ(result.first, "%E4%B8%80abc%E4%BA%8Cdef%E4%B8%89%20~-_%0D%0A");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::urlEncode("qwert.12345");
		ASSERT_EQ(result.first, "qwert.12345");
		ASSERT_EQ(result.second.size(), 0U);
	}
}

TEST(TestHttpUtils, urlDecode) {
	{
		auto result = cpv::urlDecode("%E4%B8%80abc%E4%BA%8Cdef%E4%B8%89%20~-_%0D%0A");
		ASSERT_EQ(result.first, "一abc二def三 ~-_\r\n");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::urlDecode("%E4%B8%80abc%E4%BA%8Cdef%E4%B8%89+~-_%0D%0A");
		ASSERT_EQ(result.first, "一abc二def三 ~-_\r\n");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::urlDecode("qwert.12345");
		ASSERT_EQ(result.first, "qwert.12345");
		ASSERT_EQ(result.second.size(), 0U);
	}
	{
		auto result = cpv::urlDecode("");
		ASSERT_EQ(result.first, "");
		ASSERT_EQ(result.second.size(), 0U);
	}
	{
		auto result = cpv::urlDecode("asd%E");
		ASSERT_EQ(result.first, "asd");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::urlDecode("321%");
		ASSERT_EQ(result.first, "321");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::urlDecode("%PP");
		ASSERT_EQ(result.first.size(), 1U);
		ASSERT_EQ(result.first.at(0), '\x00');
		ASSERT_EQ(result.second.size(), result.first.size());
	}
}

TEST(TestHttpUtils, htmlEncode) {
	{
		auto result = cpv::htmlEncode("&一<abc>二'def'三\"& ~-_\"\r\n&");
		ASSERT_EQ(result.first, "&amp;一&lt;abc&gt;二&#x27;def&#x27;三&quot;&amp; ~-_&quot;\r\n&amp;");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlEncode("qwert.12345");
		ASSERT_EQ(result.first, "qwert.12345");
		ASSERT_EQ(result.second.size(), 0U);
	}
}

// Notice: cases with incorrect format are for ASAN checking, their results are not guaranteed
TEST(TestHttpUtils, htmlDecode) {
	{
		auto result = cpv::htmlDecode("&amp;一&lt;abc&gt;二&#x27;def&#x27;三&quot;&amp; ~-_&quot;\r\n&amp;");
		ASSERT_EQ(result.first, "&一<abc>二'def'三\"& ~-_\"\r\n&");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("qwert.12345");
		ASSERT_EQ(result.first, "qwert.12345");
		ASSERT_EQ(result.second.size(), 0U);
	}
	{
		auto result = cpv::htmlDecode("");
		ASSERT_EQ(result.first, "");
		ASSERT_EQ(result.second.size(), 0U);
	}
	{
		auto result = cpv::htmlDecode("&quot;&#x4e00;+&#x4e8c;=&#X4e09;&QUOT;");
		ASSERT_EQ(result.first, "\"一+二=三\"");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("&#x1D56B;&#x7e;&#120171;");
		ASSERT_EQ(result.first, "𝕫~𝕫");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&#xpp;c");
		ASSERT_EQ(result.first, "abc");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&#x;c");
		ASSERT_EQ(result.first, std::string_view("ab\x00""c", 4));
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&#;c");
		ASSERT_EQ(result.first, "abc");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&#x200");
		ASSERT_EQ(result.first, "ab ");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&#x1");
		ASSERT_EQ(result.first, std::string_view("ab\x00", 3));
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&#x");
		ASSERT_EQ(result.first, "ab");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&#");
		ASSERT_EQ(result.first, "ab");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&;");
		ASSERT_EQ(result.first, "ab");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&#");
		ASSERT_EQ(result.first, "ab");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
	{
		auto result = cpv::htmlDecode("ab&");
		ASSERT_EQ(result.first, "ab");
		ASSERT_EQ(result.second.size(), result.first.size());
	}
}

TEST(TestHttpUtils, getMimeType) {
	ASSERT_EQ(cpv::getMimeType("zip"), "application/zip");
	ASSERT_EQ(cpv::getMimeType("./path/some.test.json"), "application/json");
	ASSERT_EQ(cpv::getMimeType("filename.unknown"), "application/octet-stream");
}

