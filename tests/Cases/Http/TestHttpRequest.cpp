#include <type_traits>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Http/HttpRequest.hpp>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestHttpRequest, basic) {
	cpv::HttpRequest request;
	request.setMethod("GET");
	request.setUrl("/test");
	request.setVersion("HTTP/1.1");
	seastar::temporary_buffer buf("abc asd", 7);
	std::string_view key(buf.get(), 3);
	std::string_view value(buf.get() + 4, 3);
	request.addUnderlyingBuffer(std::move(buf));
	request.setHeader(key, value);
	request.setBodyStream(
		cpv::makeObject<cpv::StringInputStream>("test body").cast<cpv::InputStreamBase>());
	return seastar::do_with(std::move(request), [] (auto& request) {
		return cpv::extensions::readAll(request.getBodyStream()).then([&request] (auto&& str) {
			ASSERT_EQ(request.getMethod(), "GET");
			ASSERT_EQ(request.getUrl(), "/test");
			ASSERT_EQ(request.getVersion(), "HTTP/1.1");
			ASSERT_EQ(request.getHeaders().getHeader("abc"), "asd");
			ASSERT_EQ(request.getUnderlyingBuffers().size(), 1U);
			ASSERT_EQ(std::string(
				request.getUnderlyingBuffers().at(0).get(),
				request.getUnderlyingBuffers().at(0).size()), "abc asd");
			ASSERT_EQ(str, "test body");
		});
	});
}

TEST(TestHttpRequest, setHeaderWithInteger) {
	cpv::HttpRequest request;
	request.setHeader("Test-First", 123);
	request.setHeader("Test-Second", 987654321);
	request.setHeader("Test-Third", -321);
	ASSERT_EQ(request.getHeaders().getHeader("Test-First"), "123");
	ASSERT_EQ(request.getHeaders().getHeader("Test-Second"), "987654321");
	ASSERT_EQ(request.getHeaders().getHeader("Test-Third"), "-321");
	ASSERT_FALSE(request.getUnderlyingBuffers().empty());
}

TEST(TestHttpRequest, headersBasic) {
	cpv::HttpRequest request;
	request.setHeader(cpv::constants::Host, "TestHost");
	request.setHeader(cpv::constants::ContentType, "TestContentType");
	request.setHeader(cpv::constants::ContentLength, "TestContentLength");
	request.setHeader(cpv::constants::Connection, "TestConnection");
	request.setHeader(cpv::constants::Pragma, "TestPragma");
	request.setHeader(cpv::constants::CacheControl, "TestCacheControl");
	request.setHeader(cpv::constants::UpgradeInsecureRequests, "TestUpgradeInsecureRequests");
	request.setHeader(cpv::constants::DNT, "TestDNT");
	request.setHeader(cpv::constants::UserAgent, "TestUserAgent");
	request.setHeader(cpv::constants::Accept, "TestAccept");
	request.setHeader(cpv::constants::AcceptEncoding, "TestAcceptEncoding");
	request.setHeader(cpv::constants::AcceptLanguage, "TestAcceptLanguage");
	request.setHeader(cpv::constants::Cookie, "TestCookie");
	request.setHeader(cpv::constants::XRequestedWith, "TestXRequestedWith");
	request.setHeader("Addition", "TestAddition");
	
	auto& headers = request.getHeaders();
	ASSERT_EQ(headers.getHeader(cpv::constants::Host), "TestHost");
	ASSERT_EQ(headers.getHeader(cpv::constants::ContentType), "TestContentType");
	ASSERT_EQ(headers.getHeader(cpv::constants::ContentLength), "TestContentLength");
	ASSERT_EQ(headers.getHeader(cpv::constants::Connection), "TestConnection");
	ASSERT_EQ(headers.getHeader(cpv::constants::Pragma), "TestPragma");
	ASSERT_EQ(headers.getHeader(cpv::constants::CacheControl), "TestCacheControl");
	ASSERT_EQ(headers.getHeader(cpv::constants::UpgradeInsecureRequests), "TestUpgradeInsecureRequests");
	ASSERT_EQ(headers.getHeader(cpv::constants::DNT), "TestDNT");
	ASSERT_EQ(headers.getHeader(cpv::constants::UserAgent), "TestUserAgent");
	ASSERT_EQ(headers.getHeader(cpv::constants::Accept), "TestAccept");
	ASSERT_EQ(headers.getHeader(cpv::constants::AcceptEncoding), "TestAcceptEncoding");
	ASSERT_EQ(headers.getHeader(cpv::constants::AcceptLanguage), "TestAcceptLanguage");
	ASSERT_EQ(headers.getHeader(cpv::constants::Cookie), "TestCookie");
	ASSERT_EQ(headers.getHeader(cpv::constants::XRequestedWith), "TestXRequestedWith");
	ASSERT_EQ(headers.getHeader("Addition"), "TestAddition");
	ASSERT_TRUE(headers.getHeader("NotExists").empty());
	
	ASSERT_EQ(headers.getHost(), "TestHost");
	ASSERT_EQ(headers.getContentType(), "TestContentType");
	ASSERT_EQ(headers.getContentLength(), "TestContentLength");
	ASSERT_EQ(headers.getConnection(), "TestConnection");
	ASSERT_EQ(headers.getPragma(), "TestPragma");
	ASSERT_EQ(headers.getCacheControl(), "TestCacheControl");
	ASSERT_EQ(headers.getUpgradeInsecureRequests(), "TestUpgradeInsecureRequests");
	ASSERT_EQ(headers.getDNT(), "TestDNT");
	ASSERT_EQ(headers.getUserAgent(), "TestUserAgent");
	ASSERT_EQ(headers.getAccept(), "TestAccept");
	ASSERT_EQ(headers.getAcceptEncoding(), "TestAcceptEncoding");
	ASSERT_EQ(headers.getAcceptLanguage(), "TestAcceptLanguage");
	ASSERT_EQ(headers.getCookie(), "TestCookie");
	ASSERT_EQ(headers.getXRequestedWith(), "TestXRequestedWith");
	
	headers.removeHeader(cpv::constants::Host);
	headers.removeHeader("Addition");
	ASSERT_TRUE(headers.getHeader(cpv::constants::Host).empty());
	ASSERT_TRUE(headers.getHeader("Addition").empty());
}

TEST(TestHttpRequest, headersForeach) {
	cpv::HttpRequest request;
	auto& headers = request.getHeaders();
	headers.setHost("TestHost");
	headers.setContentType("TestContentType");
	headers.setContentLength("TestContentLength");
	headers.setConnection("TestConnection");
	headers.setPragma("TestPragma");
	headers.setCacheControl("TestCacheControl");
	headers.setUpgradeInsecureRequests("TestUpgradeInsecureRequests");
	headers.setDNT("TestDNT");
	headers.setUserAgent("TestUserAgent");
	headers.setAccept("TestAccept");
	headers.setAcceptEncoding("TestAcceptEncoding");
	headers.setAcceptLanguage("TestAcceptLanguage");
	headers.setCookie("TestCookie");
	headers.setXRequestedWith("TestXRequestedWith");
	headers.setHeader("AdditionA", "TestAdditionA");
	headers.setHeader("AdditionB", "TestAdditionB");
	headers.setHeader("AdditionC", "TestAdditionC");
	std::string content;
	headers.foreach([&content] (const auto& key, const auto& value) {
		content.append(key).append(": ").append(value).append("\r\n");
	});
	ASSERT_EQ(content,
		"Host: TestHost\r\n"
		"Content-Type: TestContentType\r\n"
		"Content-Length: TestContentLength\r\n"
		"Connection: TestConnection\r\n"
		"Pragma: TestPragma\r\n"
		"Cache-Control: TestCacheControl\r\n"
		"Upgrade-Insecure-Requests: TestUpgradeInsecureRequests\r\n"
		"DNT: TestDNT\r\n"
		"User-Agent: TestUserAgent\r\n"
		"Accept: TestAccept\r\n"
		"Accept-Encoding: TestAcceptEncoding\r\n"
		"Accept-Language: TestAcceptLanguage\r\n"
		"Cookie: TestCookie\r\n"
		"X-Requested-With: TestXRequestedWith\r\n"
		"AdditionA: TestAdditionA\r\n"
		"AdditionB: TestAdditionB\r\n"
		"AdditionC: TestAdditionC\r\n");
}

TEST(TestHttpRequest, headersNotConstructible) {
	ASSERT_FALSE(std::is_constructible_v<cpv::HttpRequestHeaders>);
	ASSERT_FALSE(std::is_copy_constructible_v<cpv::HttpRequestHeaders>);
	ASSERT_FALSE(std::is_move_constructible_v<cpv::HttpRequestHeaders>);
	ASSERT_FALSE(std::is_copy_assignable_v<cpv::HttpRequestHeaders>);
	ASSERT_FALSE(std::is_move_assignable_v<cpv::HttpRequestHeaders>);
}

