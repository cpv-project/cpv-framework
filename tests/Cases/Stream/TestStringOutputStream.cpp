#include <array>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Object.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestStringOutputStream, all) {
	return seastar::do_with(
		cpv::StringOutputStream(),
		seastar::make_lw_shared<std::string>("test "),
		[] (auto& stream, auto& str) {
		stream.reset(str);
		return stream.write("first ", 6).then([&stream] {
			return stream.write("second", 6);
		}).then([&str] {
			ASSERT_EQ(*str, "test first second");
		});
	});
}

