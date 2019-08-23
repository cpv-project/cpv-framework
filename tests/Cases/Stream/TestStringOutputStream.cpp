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
		return stream.write(cpv::Packet("first ")).then([&stream] {
			cpv::Packet p;
			p.append("second ").append(seastar::temporary_buffer<char>("third", 5));
			return stream.write(std::move(p));
		}).then([&str] {
			ASSERT_EQ(*str, "test first second third");
		});
	});
}

