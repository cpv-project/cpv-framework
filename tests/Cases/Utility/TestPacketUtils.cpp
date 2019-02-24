#include <CPVFramework/Utility/PacketUtils.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestPacketUtils, append) {
	using namespace cpv;
	
	std::string_view first("first", 5);
	char second[] = "second";
	seastar::temporary_buffer third("third", 5);
	const char* thirdPtr = third.get();
	char last[] = "\r\n";
	
	seastar::net::packet packet;
	packet << first << second << std::move(third);
	packet << last;
	ASSERT_EQ(packet.nr_frags(), 4U);
	ASSERT_EQ(static_cast<const void*>(packet.frag(0).base), static_cast<const void*>(first.data()));
	ASSERT_EQ(packet.frag(0).size, 5U);
	ASSERT_EQ(static_cast<const void*>(packet.frag(1).base), static_cast<const void*>(second));
	ASSERT_EQ(packet.frag(1).size, 6U);
	ASSERT_EQ(static_cast<const void*>(packet.frag(2).base), static_cast<const void*>(thirdPtr));
	ASSERT_EQ(packet.frag(2).size, 5U);
	ASSERT_EQ(static_cast<const void*>(packet.frag(3).base), static_cast<const void*>(last));
	ASSERT_EQ(packet.frag(3).size, 2U);
}

