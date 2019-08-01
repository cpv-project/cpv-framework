#include <seastar/core/future-util.hh>
#include <CPVFramework/Http/HttpResponse.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestHttpResponse, all) {
	cpv::HttpResponse response;
	response.setVersion("HTTP/1.1");
	response.setStatusCode("404");
	response.setStatusMessage("Not Found");
	seastar::temporary_buffer buf("abc asd", 7);
	std::string_view key(buf.get(), 3);
	std::string_view value(buf.get() + 4, 3);
	response.addUnderlyingBuffer(std::move(buf));
	response.setHeader(key, value);
	auto str = seastar::make_lw_shared<std::string>();
	response.setBodyStream(cpv::makeObject<cpv::StringOutputStream>(str).cast<cpv::OutputStreamBase>());
	return seastar::do_with(std::move(response), std::move(str),
		[] (auto& response, auto& str) {
		return cpv::extensions::writeAll(response.getBodyStream(), "test body").then([&response, &str] {
			ASSERT_EQ(response.getVersion(), "HTTP/1.1");
			ASSERT_EQ(response.getStatusCode(), "404");
			ASSERT_EQ(response.getStatusMessage(), "Not Found");
			ASSERT_EQ(response.getHeaders().getHeader("abc"), "asd");
			ASSERT_EQ(response.getUnderlyingBuffers().size(), 1U);
			ASSERT_EQ(std::string(
				response.getUnderlyingBuffers().at(0).get(),
				response.getUnderlyingBuffers().at(0).size()), "abc asd");
			ASSERT_EQ(*str, "test body");
		});
	});
}

TEST(TestHttpResponse, setHeaderWithInteger) {
	cpv::HttpResponse response;
	response.setHeader("Test-First", 123);
	response.setHeader("Test-Second", 987654321);
	response.setHeader("Test-Third", -321);
	ASSERT_EQ(response.getHeaders().getHeader("Test-First"), "123");
	ASSERT_EQ(response.getHeaders().getHeader("Test-Second"), "987654321");
	ASSERT_EQ(response.getHeaders().getHeader("Test-Third"), "-321");
	ASSERT_FALSE(response.getUnderlyingBuffers().empty());
}

TEST(TestHttpResponse, headersBasic) {
	cpv::HttpResponse response;
	response.setHeader(cpv::constants::Date, "TestDate");
	response.setHeader(cpv::constants::ContentType, "TestContentType");
	response.setHeader(cpv::constants::ContentLength, "TestContentLength");
	response.setHeader(cpv::constants::ContentEncoding, "TestContentEncoding");
	response.setHeader(cpv::constants::TransferEncoding, "TestTransferEncoding");
	response.setHeader(cpv::constants::Connection, "TestConnection");
	response.setHeader(cpv::constants::Server, "TestServer");
	response.setHeader(cpv::constants::Vary, "TestVary");
	response.setHeader(cpv::constants::ETag, "TestETag");
	response.setHeader(cpv::constants::CacheControl, "TestCacheControl");
	response.setHeader(cpv::constants::SetCookie, "TestSetCookie");
	response.setHeader(cpv::constants::Expires, "TestExpires");
	response.setHeader(cpv::constants::LastModified, "TestLastModified");
	response.setHeader("Addition", "TestAddition");
	
	auto& headers = response.getHeaders();
	ASSERT_EQ(headers.getHeader(cpv::constants::Date), "TestDate");
	ASSERT_EQ(headers.getHeader(cpv::constants::ContentType), "TestContentType");
	ASSERT_EQ(headers.getHeader(cpv::constants::ContentLength), "TestContentLength");
	ASSERT_EQ(headers.getHeader(cpv::constants::ContentEncoding), "TestContentEncoding");
	ASSERT_EQ(headers.getHeader(cpv::constants::TransferEncoding), "TestTransferEncoding");
	ASSERT_EQ(headers.getHeader(cpv::constants::Connection), "TestConnection");
	ASSERT_EQ(headers.getHeader(cpv::constants::Server), "TestServer");
	ASSERT_EQ(headers.getHeader(cpv::constants::Vary), "TestVary");
	ASSERT_EQ(headers.getHeader(cpv::constants::ETag), "TestETag");
	ASSERT_EQ(headers.getHeader(cpv::constants::CacheControl), "TestCacheControl");
	ASSERT_EQ(headers.getHeader(cpv::constants::SetCookie), "TestSetCookie");
	ASSERT_EQ(headers.getHeader(cpv::constants::Expires), "TestExpires");
	ASSERT_EQ(headers.getHeader(cpv::constants::LastModified), "TestLastModified");
	ASSERT_EQ(headers.getHeader("Addition"), "TestAddition");
	ASSERT_TRUE(headers.getHeader("NotExists").empty());
	
	ASSERT_EQ(headers.getDate(), "TestDate");
	ASSERT_EQ(headers.getContentType(), "TestContentType");
	ASSERT_EQ(headers.getContentLength(), "TestContentLength");
	ASSERT_EQ(headers.getContentEncoding(), "TestContentEncoding");
	ASSERT_EQ(headers.getTransferEncoding(), "TestTransferEncoding");
	ASSERT_EQ(headers.getConnection(), "TestConnection");
	ASSERT_EQ(headers.getServer(), "TestServer");
	ASSERT_EQ(headers.getVary(), "TestVary");
	ASSERT_EQ(headers.getETag(), "TestETag");
	ASSERT_EQ(headers.getCacheControl(), "TestCacheControl");
	ASSERT_EQ(headers.getSetCookie(), "TestSetCookie");
	ASSERT_EQ(headers.getExpires(), "TestExpires");
	ASSERT_EQ(headers.getLastModified(), "TestLastModified");
	
	headers.removeHeader(cpv::constants::Date);
	headers.removeHeader("Addition");
	ASSERT_TRUE(headers.getHeader(cpv::constants::Date).empty());
	ASSERT_TRUE(headers.getHeader("Addition").empty());
}

TEST(TestHttpResponse, headersForeach) {
	cpv::HttpResponse response;
	auto& headers = response.getHeaders();
	headers.setDate("TestDate");
	headers.setContentType("TestContentType");
	headers.setContentLength("TestContentLength");
	headers.setContentEncoding("TestContentEncoding");
	headers.setTransferEncoding("TestTransferEncoding");
	headers.setConnection("TestConnection");
	headers.setServer("TestServer");
	headers.setVary("TestVary");
	headers.setETag("TestETag");
	headers.setCacheControl("TestCacheControl");
	headers.setSetCookie("TestSetCookie");
	headers.setExpires("TestExpires");
	headers.setLastModified("TestLastModified");
	headers.setHeader("AdditionA", "TestAdditionA");
	headers.setHeader("AdditionB", "TestAdditionB");
	headers.setHeader("AdditionC", "TestAdditionC");
	std::string content;
	headers.foreach([&content] (const auto& key, const auto& value) {
		content.append(key).append(": ").append(value).append("\r\n");
	});
	ASSERT_EQ(content,
		"Date: TestDate\r\n"
		"Content-Type: TestContentType\r\n"
		"Content-Length: TestContentLength\r\n"
		"Content-Encoding: TestContentEncoding\r\n"
		"Transfer-Encoding: TestTransferEncoding\r\n"
		"Connection: TestConnection\r\n"
		"Server: TestServer\r\n"
		"Vary: TestVary\r\n"
		"ETag: TestETag\r\n"
		"Cache-Control: TestCacheControl\r\n"
		"SetCookie: TestSetCookie\r\n"
		"Expires: TestExpires\r\n"
		"Last-Modified: TestLastModified\r\n"
		"AdditionA: TestAdditionA\r\n"
		"AdditionB: TestAdditionB\r\n"
		"AdditionC: TestAdditionC\r\n");
}

TEST(TestHttpResponse, headersNotConstructible) {
	ASSERT_FALSE(std::is_constructible_v<cpv::HttpResponseHeaders>);
	ASSERT_FALSE(std::is_copy_constructible_v<cpv::HttpResponseHeaders>);
	ASSERT_FALSE(std::is_move_constructible_v<cpv::HttpResponseHeaders>);
	ASSERT_FALSE(std::is_copy_assignable_v<cpv::HttpResponseHeaders>);
	ASSERT_FALSE(std::is_move_assignable_v<cpv::HttpResponseHeaders>);
}

