#include <array>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Stream/PacketOutputStream.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(TestPacketOutputStream, all) {
	return seastar::do_with(
		cpv::PacketOutputStream(),
		seastar::make_lw_shared<cpv::Packet>("test "),
		[] (auto& stream, auto& packet) {
		stream.reset(packet);
		return stream.write(cpv::Packet("first ")).then([&stream] {
			cpv::Packet p;
			p.append("second ").append(seastar::temporary_buffer<char>("third", 5));
			return stream.write(std::move(p));
		}).then([&packet] {
			ASSERT_EQ(packet->toString(), "test first second third");
		});
	});
}

