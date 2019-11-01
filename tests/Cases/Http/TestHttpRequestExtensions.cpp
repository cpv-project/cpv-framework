#include <CPVFramework/Http/HttpRequestExtensions.hpp>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(TestHttpRequestExtensions, readBodyStream) {
	return seastar::do_with(
		cpv::HttpRequest(),
		std::string(),
		[] (auto& request, auto& source) {
		for (std::size_t i = 0; i < 9000; ++i) {
			source.append(1, static_cast<char>(i));
		}
		request.setBodyStream(cpv::makeReusable<cpv::StringInputStream>(
			std::string(source)).cast<cpv::InputStreamBase>());
		return cpv::extensions::readBodyStream(request).then([&source] (std::string str) {
			ASSERT_EQ(str, source);
		}).then([&request] {
			request.setBodyStream(cpv::Reusable<cpv::InputStreamBase>());
			return cpv::extensions::readBodyStream(request);
		}).then([] (std::string str) {
			ASSERT_TRUE(str.empty());
		});
	});
}

TEST_FUTURE(TestHttpRequestExtensions, readBodyStreamAsBuffer) {
	return seastar::do_with(
		cpv::HttpRequest(),
		std::string(),
		[] (auto& request, auto& source) {
		for (std::size_t i = 0; i < 9000; ++i) {
			source.append(1, static_cast<char>(i));
		}
		request.setBodyStream(cpv::makeReusable<cpv::StringInputStream>(
			std::string(source)).cast<cpv::InputStreamBase>());
		return cpv::extensions::readBodyStreamAsBuffer(request).then(
			[&source] (seastar::temporary_buffer<char> buf) {
			ASSERT_EQ(std::string_view(buf.get(), buf.size()), source);
		}).then([&request] {
			request.setBodyStream(cpv::Reusable<cpv::InputStreamBase>());
			return cpv::extensions::readBodyStreamAsBuffer(request);
		}).then([] (seastar::temporary_buffer<char> buf) {
			ASSERT_EQ(buf.size(), 0U);
		});
	});
}

// TEST_FUTURE(TestHttpRequestExtensions, readBodyStreamAsJson) {
// }

// TEST_FUTURE(TestHttpRequestExtensions, readBodyStreamAsForm) {
// }

TEST(TestHttpRequestExtensions, getParameter_size_t) {
	cpv::HttpRequest request;
	request.setUrl("/test/abc/123");
	ASSERT_EQ(cpv::extensions::getParameter(request, 0), "test");
	ASSERT_EQ(cpv::extensions::getParameter(request, 1), "abc");
	ASSERT_EQ(cpv::extensions::getParameter(request, 2), "123");
	ASSERT_EQ(cpv::extensions::getParameter(request, 3), "");
}

TEST(TestHttpRequestExtensions, getParameter_string_view) {
	cpv::HttpRequest request;
	request.setUrl("/test?key=123&value=321");
	ASSERT_EQ(cpv::extensions::getParameter(request, "key"), "123");
	ASSERT_EQ(cpv::extensions::getParameter(request, "value"), "321");
	ASSERT_EQ(cpv::extensions::getParameter(request, "non-exists"), "");
}

