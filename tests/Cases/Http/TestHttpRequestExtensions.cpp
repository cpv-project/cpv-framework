#include <CPVFramework/Http/HttpRequestExtensions.hpp>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(HttpRequestExtensions, readBodyStream) {
	return seastar::do_with(
		cpv::HttpRequest(),
		std::string(),
		[] (auto& request, auto& source) {
		for (std::size_t i = 0; i < 9000; ++i) {
			source.append(1, static_cast<char>(i));
		}
		request.setBodyStream(cpv::makeReusable<cpv::StringInputStream>(
			cpv::SharedString(source)).cast<cpv::InputStreamBase>());
		return cpv::extensions::readBodyStream(request).then([&source] (cpv::SharedString str) {
			ASSERT_EQ(str, source);
		}).then([&request] {
			request.setBodyStream(cpv::Reusable<cpv::InputStreamBase>());
			return cpv::extensions::readBodyStream(request);
		}).then([] (cpv::SharedString str) {
			ASSERT_TRUE(str.empty());
		});
	});
}

// TEST_FUTURE(HttpRequestExtensions, readBodyStreamAsJson) {
// }

// TEST_FUTURE(HttpRequestExtensions, readBodyStreamAsForm) {
// }

