#include <seastar/core/reactor.hh>
#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Application/Modules/LoggingModule.hpp>
#include <CPVFramework/Application/Modules/HttpServerModule.hpp>
#include <CPVFramework/Utility/SocketHolder.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST_FUTURE(SocketHolder, all) {
	cpv::Application application;
	application.add<cpv::LoggingModule>([] (auto& module) {
		module.setLogger(cpv::Logger::createNoop());
	});
	application.add<cpv::HttpServerModule>([] (auto& module) {
		ASSERT_EQ(module.getConfig().getListenAddresses().size(), 1U);
		ASSERT_EQ(module.getConfig().getListenAddresses().at(0), "127.0.0.1:8000");
		module.getConfig().setListenAddresses({
			cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
			cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
		});
	});
	return application.start().then([application] () mutable {
		seastar::socket_address address(seastar::ipv4_addr(HTTP_SERVER_1_IP, HTTP_SERVER_1_PORT));
		return seastar::engine().net().connect(address).then([] (seastar::connected_socket&& socket) {
			cpv::SocketHolder holder;
			ASSERT_FALSE(holder.isConnected());
			holder = cpv::SocketHolder(std::move(socket));
			ASSERT_TRUE(holder.isConnected());
		}).then([application] () mutable {
			return application.stop();
		});
	});
}

