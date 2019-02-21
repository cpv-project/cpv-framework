#include <array>
#include <string_view>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/BufferInputStream.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestBufferInputStream, all) {
	return seastar::do_with (
		cpv::BufferInputStream(),
		[] (auto& stream) {
		stream.reset(seastar::temporary_buffer<char>("test data", 9));
		return stream.read().then([&stream] (auto&& result) {
			ASSERT_EQ(result.view(), "test data");
			ASSERT_TRUE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.view(), "");
			ASSERT_TRUE(result.isEnd);
		});
	});
}
