#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(TestInputStreamExtensions, readAll) {
	return seastar::do_with(
		cpv::StringInputStream(),
		std::string(),
		[] (auto& stream, auto& source) {
		for (std::size_t i = 0; i < 9000; ++i) {
			source.append(1, static_cast<char>(i));
		}
		stream.reset(std::string(source));
		return cpv::extensions::readAll(stream).then([&source] (auto&& str) {
			ASSERT_EQ(str, source);
		});
	});
}

TEST_FUTURE(TestInputStreamExtensions, readAll_ptr) {
	return seastar::do_with(
		cpv::makeReusable<cpv::StringInputStream>("test body").cast<cpv::InputStreamBase>(),
		cpv::Reusable<cpv::InputStreamBase>(),
		[] (auto& stream, auto& nullStream) {
		return cpv::extensions::readAll(stream).then([] (auto&& str) {
			ASSERT_EQ(str, "test body");
		}).then([&nullStream] {
			return cpv::extensions::readAll(nullStream);
		}).then([] (auto&& str) {
			ASSERT_EQ(str, "");
		});
	});
}

