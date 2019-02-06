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
	std::string_view bufView = response.addUnderlyingBuffer(std::move(buf));
	std::string_view key = bufView.substr(0, 3);
	std::string_view value = bufView.substr(4, 3);
	response.setHeader(key, value);
	auto str = seastar::make_lw_shared<std::string>();
	response.setBodyStream(cpv::makeObject<cpv::StringOutputStream>(str).cast<cpv::OutputStreamBase>());
	return seastar::do_with(std::move(response), std::move(str),
		[] (auto& response, auto& str) {
		return cpv::extensions::writeAll(response.getBodyStream(), "test body").then([&response, &str] {
			ASSERT_EQ(response.getVersion(), "HTTP/1.1");
			ASSERT_EQ(response.getStatusCode(), "404");
			ASSERT_EQ(response.getStatusMessage(), "Not Found");
			ASSERT_EQ(response.getHeaders().size(), 1U);
			ASSERT_EQ(response.getHeaders().at("abc"), "asd");
			ASSERT_EQ(*str, "test body");
		});
	});
}

