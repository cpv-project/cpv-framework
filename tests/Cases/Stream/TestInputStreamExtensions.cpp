#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <CPVFramework/Stream/BuffersInputStream.hpp>
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
		return cpv::extensions::readAll(stream).then([&source] (std::string str) {
			ASSERT_EQ(str, source);
		});
	});
}

TEST_FUTURE(TestInputStreamExtensions, readAll_ptr) {
	return seastar::do_with(
		cpv::makeReusable<cpv::StringInputStream>("test body").cast<cpv::InputStreamBase>(),
		cpv::Reusable<cpv::InputStreamBase>(),
		[] (auto& stream, auto& nullStream) {
		return cpv::extensions::readAll(stream).then([] (std::string str) {
			ASSERT_EQ(str, "test body");
		}).then([&nullStream] {
			return cpv::extensions::readAll(nullStream);
		}).then([] (std::string str) {
			ASSERT_EQ(str, "");
		});
	});
}

TEST_FUTURE(TestInputStreamExtensions, readAll_multipleSegments) {
	return seastar::do_with(
		cpv::BuffersInputStream(),
		[] (auto& stream) {
		std::vector<seastar::temporary_buffer<char>> buffers;
		buffers.emplace_back(seastar::temporary_buffer<char>("first:", 6));
		buffers.emplace_back(seastar::temporary_buffer<char>("second:", 7));
		buffers.emplace_back(seastar::temporary_buffer<char>("third", 5));
		stream.reset(std::move(buffers));
		return cpv::extensions::readAll(stream).then([] (std::string str) {
			ASSERT_EQ(str, "first:second:third");
		});
	});
}

TEST_FUTURE(TestInputStreamExtensions, readAllAsBuffer) {
	return seastar::do_with(
		cpv::StringInputStream(),
		std::string(),
		[] (auto& stream, auto& source) {
		for (std::size_t i = 0; i < 9000; ++i) {
			source.append(1, static_cast<char>(i));
		}
		stream.reset(std::string(source));
		return cpv::extensions::readAllAsBuffer(stream).then(
			[&source] (seastar::temporary_buffer<char> buf) {
			ASSERT_EQ(std::string_view(buf.get(), buf.size()), source);
		});
	});
}

TEST_FUTURE(TestInputStreamExtensions, readAllAsBuffer_ptr) {
	return seastar::do_with(
		cpv::makeReusable<cpv::StringInputStream>("test body").cast<cpv::InputStreamBase>(),
		cpv::Reusable<cpv::InputStreamBase>(),
		[] (auto& stream, auto& nullStream) {
		return cpv::extensions::readAllAsBuffer(stream).then(
			[] (seastar::temporary_buffer<char> buf) {
			ASSERT_EQ(std::string_view(buf.get(), buf.size()), "test body");
		}).then([&nullStream] {
			return cpv::extensions::readAllAsBuffer(nullStream);
		}).then([] (seastar::temporary_buffer<char> buf) {
			ASSERT_EQ(buf.size(), 0U);
		});
	});
}

TEST_FUTURE(TestInputStreamExtensions, readAllAsBuffer_multipleSegments) {
	return seastar::do_with(
		cpv::BuffersInputStream(),
		[] (auto& stream) {
		std::vector<seastar::temporary_buffer<char>> buffers;
		buffers.emplace_back(seastar::temporary_buffer<char>("first:", 6));
		buffers.emplace_back(seastar::temporary_buffer<char>("second:", 7));
		buffers.emplace_back(seastar::temporary_buffer<char>("third", 5));
		stream.reset(std::move(buffers));
		return cpv::extensions::readAllAsBuffer(stream).then(
			[] (seastar::temporary_buffer<char> buf) {
			ASSERT_EQ(std::string_view(buf.get(), buf.size()), "first:second:third");
		});
	});
}

