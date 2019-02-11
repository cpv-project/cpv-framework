#include <array>
#include <string_view>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/StringViewInputStream.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestStringViewInputStream, all) {
	return seastar::do_with (
		cpv::StringViewInputStream(),
		[] (auto& stream) {
		stream.reset("test data");
		return stream.read().then([&stream] (auto&& result) {
			ASSERT_EQ(result.underlyingBuffer.size(), 0U);
			ASSERT_EQ(result.data, "test data");
			ASSERT_TRUE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.underlyingBuffer.size(), 0U);
			ASSERT_EQ(result.data, "");
			ASSERT_TRUE(result.isEnd);
		});
	});
}

