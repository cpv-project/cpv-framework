#include <array>
#include <string_view>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Utility/Object.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestStringInputStream, all) {
	return seastar::do_with(
		cpv::StringInputStream(),
		std::array<char, 5>(),
		[] (auto& stream, auto& buf) {
		stream.reset("test data");
		return stream.read(buf.data(), buf.size()).then([&stream, &buf] (auto&& result) {
			ASSERT_EQ(result.size, 5U);
			ASSERT_FALSE(result.eof);
			std::string_view str(buf.data(), result.size);
			ASSERT_EQ(str, "test ");
		}).then([&stream, &buf] {
			return stream.read(buf.data(), buf.size());
		}).then([&stream, &buf] (auto&& result) {
			ASSERT_EQ(result.size, 4U);
			ASSERT_TRUE(result.eof);
			std::string_view str(buf.data(), result.size);
			ASSERT_EQ(str, "data");
		}).then([&stream, &buf] {
			return stream.read(buf.data(), buf.size());
		}).then([&stream, &buf] (auto&& result) {
			ASSERT_EQ(result.size, 0U);
			ASSERT_TRUE(result.eof);
		});
	});
}

