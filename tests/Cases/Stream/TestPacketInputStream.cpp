#include <array>
#include <string_view>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/PacketInputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(TestPacketInputStream, all) {
	return seastar::do_with (
		cpv::PacketInputStream(),
		[] (auto& stream) {
		cpv::Packet p;
		p.append("first").append("second").append("").append("third");
		stream.reset(std::move(p));
		return stream.read().then([&stream] (auto&& result) {
			ASSERT_TRUE(stream.sizeHint().has_value());
			ASSERT_EQ(*stream.sizeHint(), 16U);
			ASSERT_EQ(result.data, "first");
			ASSERT_FALSE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.data, "second");
			ASSERT_FALSE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.data, "third");
			ASSERT_TRUE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.data, "");
			ASSERT_TRUE(result.isEnd);
		});
	});
}

TEST_FUTURE(TestPacketInputStream, empty) {
	return seastar::do_with (
		cpv::PacketInputStream(),
		[] (auto& stream) {
		cpv::Packet p;
		p.append("first").append("");
		stream.reset(std::move(p));
		return stream.read().then([&stream] (auto&& result) {
			// result.isEnd is false because PacketInputStream won't check
			// the following segments is empty or not
			ASSERT_EQ(result.data, "first");
			ASSERT_FALSE(result.isEnd);
		}).then([&stream] {
			return stream.read();
		}).then([&stream] (auto&& result) {
			ASSERT_EQ(result.data, "");
			ASSERT_TRUE(result.isEnd);
		});
	});
}

