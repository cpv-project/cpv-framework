#include <array>
#include <string_view>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/BuffersInputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(TestBuffersInputStream, all) {
	return seastar::do_with (
		cpv::BuffersInputStream(),
		[] (auto& stream) {
		std::vector<seastar::temporary_buffer<char>> buffers;
		buffers.emplace_back(seastar::temporary_buffer<char>("first", 5));
		buffers.emplace_back(seastar::temporary_buffer<char>("second", 6));
		buffers.emplace_back(seastar::temporary_buffer<char>());
		buffers.emplace_back(seastar::temporary_buffer<char>("third", 5));
		stream.reset(std::move(buffers));
		return stream.read().then([&stream] (auto&& result) {
			ASSERT_TRUE(stream.sizeHint().has_value());
			ASSERT_EQ(*stream.sizeHint(), 16U);
			ASSERT_EQ(result.view(), "first");
			ASSERT_FALSE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.view(), "second");
			ASSERT_FALSE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.view(), "third");
			ASSERT_TRUE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.view(), "");
			ASSERT_TRUE(result.isEnd);
		});
	});
}

TEST_FUTURE(TestBuffersInputStream, empty) {
	return seastar::do_with (
		cpv::BuffersInputStream(),
		[] (auto& stream) {
		std::vector<seastar::temporary_buffer<char>> buffers;
		buffers.emplace_back(seastar::temporary_buffer<char>("first", 5));
		buffers.emplace_back(seastar::temporary_buffer<char>());
		stream.reset(std::move(buffers));
		return stream.read().then([&stream] (auto&& result) {
			// result.isEnd is false because BuffersInputStream won't check
			// the following buffers is empty or not
			ASSERT_EQ(result.view(), "first");
			ASSERT_FALSE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.view(), "");
			ASSERT_TRUE(result.isEnd);
		});
	});
}

