#include <seastar/core/future-util.hh>
#include <CPVFramework/Http/HttpRequest.hpp>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestHttpRequest, all) {
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
			ASSERT_EQ(request.getHeaders().size(), 1U);
			ASSERT_EQ(request.getHeaders().at("abc"), "asd");
			ASSERT_EQ(str, "test body");
		});
	});
}
