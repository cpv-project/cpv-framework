#include <array>
#include <seastar/core/future-util.hh>
#include <seastar/core/scattered_message.hh>
#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestStringOutputStream, all) {
	return seastar::do_with(
		cpv::StringOutputStream(),
		seastar::make_lw_shared<std::string>("test "),
		[] (auto& stream, auto& str) {
		stream.reset(str);
		return stream.write(seastar::net::packet::from_static_data("first ", 6)).then([&stream] {
			seastar::scattered_message<char> msg;
			msg.append_static("second ");
			msg.append_static("third");
			return stream.write(std::move(msg).release());
		}).then([&str] {
			ASSERT_EQ(*str, "test first second third");
		});
	});
}

