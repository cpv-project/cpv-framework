#include <core/reactor.hh>
#include <CPVFramework/Utility/SocketHolder.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestSocketHolder, all) {
	// TODO: use httpd here
	/* seastar::socket_address address(seastar::ipv4_addr(HTTPD_LISTEN_IP, HTTPD_LISTEN_PORT));
	return seastar::engine().net().connect(address).then([] (seastar::connected_socket&& socket) {
		cpv::SocketHolder holder;
		ASSERT_FALSE(holder.isConnected());
		holder = cpv::SocketHolder(std::move(socket));
		ASSERT_TRUE(holder.isConnected());
	}); */
}

