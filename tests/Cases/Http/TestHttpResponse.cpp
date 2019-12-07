#include <seastar/core/future-util.hh>
#include <CPVFramework/Http/HttpResponse.hpp>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(HttpResponse, all) {
	cpv::HttpResponse response;
	response.setVersion("HTTP/1.1");
	response.setStatusCode("404");
	response.setStatusMessage("Not Found");
	response.setHeader(cpv::SharedString(std::string_view("abc")), "asd");
	auto str = seastar::make_lw_shared<cpv::SharedStringBuilder>();
	response.setBodyStream(cpv::makeReusable<cpv::StringOutputStream>(str).cast<cpv::OutputStreamBase>());
	return seastar::do_with(std::move(response), std::move(str),
		[] (auto& response, auto& str) {
		return cpv::extensions::writeAll(response.getBodyStream(), "test body").then([&response, &str] {
			ASSERT_EQ(response.getVersion(), "HTTP/1.1");
			ASSERT_EQ(response.getStatusCode(), "404");
			ASSERT_EQ(response.getStatusMessage(), "Not Found");
			ASSERT_EQ(response.getHeaders().getHeader("abc"), "asd");
			ASSERT_EQ(str->view(), "test body");
		});
	});
}

TEST(HttpResponse, headersBasic) {
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
	ASSERT_EQ(headers.getExpires(), "TestExpires");
	ASSERT_EQ(headers.getLastModified(), "TestLastModified");
	
	headers.removeHeader(cpv::constants::Date);
	headers.removeHeader("Addition");
	ASSERT_TRUE(headers.getHeader(cpv::constants::Date).empty());
	ASSERT_TRUE(headers.getHeader("Addition").empty());
}

TEST(HttpResponse, headersForeach) {
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
		"Expires: TestExpires\r\n"
		"Last-Modified: TestLastModified\r\n"
		"AdditionA: TestAdditionA\r\n"
		"AdditionB: TestAdditionB\r\n"
		"AdditionC: TestAdditionC\r\n");
}

TEST(HttpResponse, additionHeaders) {
	cpv::HttpResponse response;
	auto& headers = response.getHeaders();
	headers.addAdditionHeader("Key", "ValueA");
	headers.addAdditionHeader("Key", "ValueB");
	ASSERT_EQ(headers.getAdditionHeaders().size(), 2U);
	ASSERT_EQ(headers.getAdditionHeaders().at(0).first, "Key");
	ASSERT_EQ(headers.getAdditionHeaders().at(0).second, "ValueA");
	ASSERT_EQ(headers.getAdditionHeaders().at(1).first, "Key");
	ASSERT_EQ(headers.getAdditionHeaders().at(1).second, "ValueB");
	std::string content;
	headers.foreach([&content] (const auto& key, const auto& value) {
		content.append(key).append(": ").append(value).append("\r\n");
	});
	ASSERT_EQ(content,
		"Key: ValueA\r\n"
		"Key: ValueB\r\n");
}

TEST(HttpResponse, headersNotConstructible) {
	ASSERT_FALSE(std::is_constructible_v<cpv::HttpResponseHeaders>);
	ASSERT_FALSE(std::is_copy_constructible_v<cpv::HttpResponseHeaders>);
	ASSERT_FALSE(std::is_move_constructible_v<cpv::HttpResponseHeaders>);
	ASSERT_FALSE(std::is_copy_assignable_v<cpv::HttpResponseHeaders>);
	ASSERT_FALSE(std::is_move_assignable_v<cpv::HttpResponseHeaders>);
}

TEST(HttpResponse, headersClear) {
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
	headers.setExpires("TestExpires");
	headers.setLastModified("TestLastModified");
	headers.setHeader("Addition", "TestAddition");
	headers.clear();
	ASSERT_TRUE(headers.getDate().empty());
	ASSERT_TRUE(headers.getContentType().empty());
	ASSERT_TRUE(headers.getContentLength().empty());
	ASSERT_TRUE(headers.getContentEncoding().empty());
	ASSERT_TRUE(headers.getTransferEncoding().empty());
	ASSERT_TRUE(headers.getConnection().empty());
	ASSERT_TRUE(headers.getServer().empty());
	ASSERT_TRUE(headers.getVary().empty());
	ASSERT_TRUE(headers.getETag().empty());
	ASSERT_TRUE(headers.getCacheControl().empty());
	ASSERT_TRUE(headers.getExpires().empty());
	ASSERT_TRUE(headers.getLastModified().empty());
	ASSERT_TRUE(headers.getHeader("Addition").empty());
}

