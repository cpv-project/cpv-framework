#include <CPVFramework/Utility/Uri.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestUri, getsetclear) {
	cpv::Uri uri;
	ASSERT_EQ(uri.getProtocol(), "");
	ASSERT_EQ(uri.getHostname(), "");
	ASSERT_EQ(uri.getPort(), "");
	ASSERT_EQ(uri.getPath(), "");
	ASSERT_TRUE(uri.getPathFragments().empty());
	ASSERT_TRUE(uri.getQueryParameters().empty());

	uri.setProtocol("http");
	uri.setHostname("www.example.com");
	uri.setPort("8000");
	uri.setPath("/first/second");
	uri.getPathFragments().emplace_back("first");
	uri.getPathFragments().emplace_back("second");
	uri.setQueryParameter("a", "123");
	uri.setQueryParameter("b", "456");
	uri.setQueryParameter("c", "789");
	uri.removeQueryParameter("c");

	ASSERT_EQ(uri.getProtocol(), "http");
	ASSERT_EQ(uri.getHostname(), "www.example.com");
	ASSERT_EQ(uri.getPort(), "8000");
	ASSERT_EQ(uri.getPath(), "/first/second");
	ASSERT_EQ(uri.getPathFragments().size(), 2U);
	ASSERT_EQ(uri.getPathFragment(0), "first");
	ASSERT_EQ(uri.getPathFragment(1), "second");
	ASSERT_EQ(uri.getPathFragment(2), "");
	ASSERT_EQ(uri.getQueryParameters().size(), 2U);
	ASSERT_EQ(uri.getQueryParameter("a"), "123");
	ASSERT_EQ(uri.getQueryParameter("b"), "456");
	ASSERT_EQ(uri.getQueryParameter("c"), "");

	uri.clear();

	ASSERT_EQ(uri.getProtocol(), "");
	ASSERT_EQ(uri.getHostname(), "");
	ASSERT_EQ(uri.getPort(), "");
	ASSERT_EQ(uri.getPath(), "");
	ASSERT_TRUE(uri.getPathFragments().empty());
	ASSERT_TRUE(uri.getQueryParameters().empty());
}

TEST(TestUri, parse) {
	{
		cpv::Uri uri("http://www.example.com/index");
		ASSERT_EQ(uri.getProtocol(), "http");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/index");
		ASSERT_EQ(uri.getPathFragments().size(), 1U);
		ASSERT_EQ(uri.getPathFragments().at(0), "index");
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		std::string_view uriStr("http://www.example.com/index");
		cpv::Uri uri(seastar::temporary_buffer<char>(uriStr.data(), uriStr.size()));
		ASSERT_EQ(uri.getProtocol(), "http");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/index");
		ASSERT_EQ(uri.getPathFragments().size(), 1U);
		ASSERT_EQ(uri.getPathFragments().at(0), "index");
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("https://www.example.com:8000/index");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "8000");
		ASSERT_EQ(uri.getPath(), "/index");
		ASSERT_EQ(uri.getPathFragments().size(), 1U);
		ASSERT_EQ(uri.getPathFragments().at(0), "index");
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("https://www.example.com:8000");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "8000");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("https://www.example.com:");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("https://www.example.com");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("https://www.example%20.com");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example .com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("http://[::1]:81");
		ASSERT_EQ(uri.getProtocol(), "http");
		ASSERT_EQ(uri.getHostname(), "[::1]");
		ASSERT_EQ(uri.getPort(), "81");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("http://[::1]:81/");
		ASSERT_EQ(uri.getProtocol(), "http");
		ASSERT_EQ(uri.getHostname(), "[::1]");
		ASSERT_EQ(uri.getPort(), "81");
		ASSERT_EQ(uri.getPath(), "/");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("https://www.example.com:/");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("https://www.example.com/first/second/third");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/first/second/third");
		ASSERT_EQ(uri.getPathFragments().size(), 3U);
		ASSERT_EQ(uri.getPathFragment(0), "first");
		ASSERT_EQ(uri.getPathFragment(1), "second");
		ASSERT_EQ(uri.getPathFragment(2), "third");
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("https://www.example.com/first/second/third?a=123");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/first/second/third");
		ASSERT_EQ(uri.getPathFragments().size(), 3U);
		ASSERT_EQ(uri.getPathFragment(0), "first");
		ASSERT_EQ(uri.getPathFragment(1), "second");
		ASSERT_EQ(uri.getPathFragment(2), "third");
		ASSERT_EQ(uri.getQueryParameters().size(), 1U);
		ASSERT_EQ(uri.getQueryParameter("a"), "123");
	}
	{
		cpv::Uri uri("https://www.example.com/first/second/t%20hi+rd?a=123&b=%3c%3d%3d");
		ASSERT_EQ(uri.getProtocol(), "https");
		ASSERT_EQ(uri.getHostname(), "www.example.com");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/first/second/t hi rd");
		ASSERT_EQ(uri.getPathFragments().size(), 3U);
		ASSERT_EQ(uri.getPathFragment(0), "first");
		ASSERT_EQ(uri.getPathFragment(1), "second");
		ASSERT_EQ(uri.getPathFragment(2), "t hi rd");
		ASSERT_EQ(uri.getQueryParameters().size(), 2U);
		ASSERT_EQ(uri.getQueryParameter("a"), "123");
		ASSERT_EQ(uri.getQueryParameter("b"), "<==");
	}
	{
		cpv::Uri uri("/first/second/t%20hi+rd?a=123&b=%3c%3d%3d");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/first/second/t hi rd");
		ASSERT_EQ(uri.getPathFragments().size(), 3U);
		ASSERT_EQ(uri.getPathFragment(0), "first");
		ASSERT_EQ(uri.getPathFragment(1), "second");
		ASSERT_EQ(uri.getPathFragment(2), "t hi rd");
		ASSERT_EQ(uri.getQueryParameters().size(), 2U);
		ASSERT_EQ(uri.getQueryParameter("a"), "123");
		ASSERT_EQ(uri.getQueryParameter("b"), "<==");
	}
	{
		cpv::Uri uri("first/second/t%20hi+rd?a=123&b=%3c%3d%3d");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "first/second/t hi rd");
		ASSERT_EQ(uri.getPathFragments().size(), 3U);
		ASSERT_EQ(uri.getPathFragment(0), "first");
		ASSERT_EQ(uri.getPathFragment(1), "second");
		ASSERT_EQ(uri.getPathFragment(2), "t hi rd");
		ASSERT_EQ(uri.getQueryParameters().size(), 2U);
		ASSERT_EQ(uri.getQueryParameter("a"), "123");
		ASSERT_EQ(uri.getQueryParameter("b"), "<==");
	}
	{
		cpv::Uri uri("first//t%20hi+rd?a=123&b=%3c%3d%3d");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "first//t hi rd");
		ASSERT_EQ(uri.getPathFragments().size(), 3U);
		ASSERT_EQ(uri.getPathFragment(0), "first");
		ASSERT_EQ(uri.getPathFragment(1), "");
		ASSERT_EQ(uri.getPathFragment(2), "t hi rd");
		ASSERT_EQ(uri.getQueryParameters().size(), 2U);
		ASSERT_EQ(uri.getQueryParameter("a"), "123");
		ASSERT_EQ(uri.getQueryParameter("b"), "<==");
	}
	{
		cpv::Uri uri("/first/second/third");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/first/second/third");
		ASSERT_EQ(uri.getPathFragments().size(), 3U);
		ASSERT_EQ(uri.getPathFragment(0), "first");
		ASSERT_EQ(uri.getPathFragment(1), "second");
		ASSERT_EQ(uri.getPathFragment(2), "third");
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("/first/second/");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/first/second/");
		ASSERT_EQ(uri.getPathFragments().size(), 2U);
		ASSERT_EQ(uri.getPathFragment(0), "first");
		ASSERT_EQ(uri.getPathFragment(1), "second");
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("/");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("abc:");
		ASSERT_EQ(uri.getProtocol(), "abc");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("://");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri(":///");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("://:");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("://:/");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "/");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("?a=123&b=%20");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_EQ(uri.getQueryParameters().size(), 2U);
		ASSERT_EQ(uri.getQueryParameter("a"), "123");
		ASSERT_EQ(uri.getQueryParameter("b"), " ");
	}
	{
		cpv::Uri uri("?a=123&b=");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_EQ(uri.getQueryParameters().size(), 1U);
		ASSERT_EQ(uri.getQueryParameter("a"), "123");
		ASSERT_EQ(uri.getQueryParameter("b"), ""); // same as not exists
	}
	{
		cpv::Uri uri("?a=123");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_EQ(uri.getQueryParameters().size(), 1U);
		ASSERT_EQ(uri.getQueryParameter("a"), "123");
	}
	{
		cpv::Uri uri("?a=123=456");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_EQ(uri.getQueryParameters().size(), 1U);
		ASSERT_EQ(uri.getQueryParameter("123"), "456");
	}
	{
		cpv::Uri uri("?=");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_TRUE(uri.getQueryParameters().empty());
	}
	{
		cpv::Uri uri("?=1");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_EQ(uri.getQueryParameters().size(), 1U);
		ASSERT_EQ(uri.getQueryParameter(""), "1");
	}
	{
		cpv::Uri uri("?=1&");
		ASSERT_EQ(uri.getProtocol(), "");
		ASSERT_EQ(uri.getHostname(), "");
		ASSERT_EQ(uri.getPort(), "");
		ASSERT_EQ(uri.getPath(), "");
		ASSERT_TRUE(uri.getPathFragments().empty());
		ASSERT_EQ(uri.getQueryParameters().size(), 1U);
		ASSERT_EQ(uri.getQueryParameter(""), "1");
	}
}

TEST(TestUri, build) {
	{
		cpv::Uri uri;
		uri.setProtocol("http");
		uri.setHostname("www.example.com");
		uri.setPort("8000");
		uri.setPath("/first/second");
		uri.setQueryParameter("a", "123");
		uri.setQueryParameter("b", "456");
		cpv::Packet packet;
		uri.build(packet);
		ASSERT_EQ(cpv::joinString("", packet), "http://www.example.com:8000/first/second?a=123&b=456");
	}
	{
		cpv::Uri uri;
		uri.setProtocol("http");
		uri.setHostname("www.example.com");
		uri.setPort("8000");
		uri.setPath("/first/second");
		uri.getPathFragments().emplace_back("first");
		uri.getPathFragments().emplace_back("second_modified"); // perfer path fragments
		cpv::Packet packet;
		uri.build(packet);
		ASSERT_EQ(cpv::joinString("", packet), "http://www.example.com:8000/first/second_modified");
	}
	{
		cpv::Uri uri;
		uri.setHostname("www.example.com");
		uri.setPort("8000");
		uri.setPath("/first/secon d");
		uri.setQueryParameter("a ", "123");
		uri.setQueryParameter("b", "45 6");
		cpv::Packet packet;
		uri.build(packet);
		ASSERT_EQ(cpv::joinString("", packet), "/first/secon%20d?a%20=123&b=45%206");
	}
	{
		cpv::Uri uri;
		uri.getPathFragments().emplace_back("first");
		uri.getPathFragments().emplace_back("second ");
		uri.setQueryParameter("a ", "123");
		uri.setQueryParameter("b", "45 6");
		cpv::Packet packet;
		uri.build(packet);
		ASSERT_EQ(cpv::joinString("", packet), "/first/second%20?a%20=123&b=45%206");
	}
	{
		cpv::Uri uri;
		uri.setPath("/first/second");
		cpv::Packet packet;
		uri.build(packet);
		ASSERT_EQ(cpv::joinString("", packet), "/first/second");
	}
	{
		cpv::Uri uri;
		uri.setQueryParameter("a ", "123");
		uri.setQueryParameter("b", "45 6");
		cpv::Packet packet;
		uri.build(packet);
		ASSERT_EQ(cpv::joinString("", packet), "?a%20=123&b=45%206");
	}
	{
		cpv::Uri uri;
		cpv::Packet packet;
		uri.build(packet);
		ASSERT_EQ(cpv::joinString("", packet), "");
	}
}

