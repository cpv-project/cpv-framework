#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(HttpServerConfiguration, getset) {
	cpv::HttpServerConfiguration configuration;
	ASSERT_TRUE(configuration.getListenAddresses().empty());
	ASSERT_EQ(configuration.getMaxInitialRequestBytes(), 524288U);
	ASSERT_EQ(configuration.getMaxInitialRequestPackets(), 512U);
	ASSERT_EQ(configuration.getRequestTimeout().count(), 60000U);
	ASSERT_EQ(configuration.getRequestQueueSize(), 100U);
	ASSERT_EQ(configuration.getRequestBodyQueueSize(), 50U);
	
	configuration.setListenAddresses({ "127.0.0.1:80", "0.0.0.0:1080" });
	configuration.setMaxInitialRequestBytes(524290);
	configuration.setMaxInitialRequestPackets(513);
	configuration.setRequestTimeout(std::chrono::milliseconds(60001));
	configuration.setRequestQueueSize(101);
	configuration.setRequestBodyQueueSize(51);
	
	ASSERT_EQ(configuration.getListenAddresses().size(), 2U);
	ASSERT_EQ(configuration.getListenAddresses().at(0), "127.0.0.1:80");
	ASSERT_EQ(configuration.getListenAddresses().at(1), "0.0.0.0:1080");
	ASSERT_EQ(configuration.getMaxInitialRequestBytes(), 524290U);
	ASSERT_EQ(configuration.getMaxInitialRequestPackets(), 513U);
	ASSERT_EQ(configuration.getRequestTimeout().count(), 60001U);
	ASSERT_EQ(configuration.getRequestQueueSize(), 101U);
	ASSERT_EQ(configuration.getRequestBodyQueueSize(), 51U);
}

TEST(HttpServerConfiguration, loadJson) {
	{
		cpv::HttpServerConfiguration configuration;
		cpv::SharedString json("{}");
		auto error = cpv::deserializeJson(configuration, json);
		if (error.has_value()) {
			std::cout << error->what() << std::endl;
		}
		ASSERT_FALSE(error.has_value());
		ASSERT_TRUE(configuration.getListenAddresses().empty());
		ASSERT_EQ(configuration.getMaxInitialRequestBytes(), 524288U);
		ASSERT_EQ(configuration.getMaxInitialRequestPackets(), 512U);
		ASSERT_EQ(configuration.getRequestTimeout().count(), 60000U);
		ASSERT_EQ(configuration.getRequestQueueSize(), 100U);
		ASSERT_EQ(configuration.getRequestBodyQueueSize(), 50U);
	}
	{
		cpv::HttpServerConfiguration configuration;
		cpv::SharedString json(std::string_view(R"(
			{
				"listenAddresses": [ "127.0.0.1:80", "0.0.0.0:1080" ],
				"maxInitialRequestBytes": 524290,
				"maxInitialRequestPackets": 513,
				"requestTimeout": 60001,
				"requestQueueSize": 101,
				"requestBodyQueueSize": 51
			}
		)"));
		auto error = cpv::deserializeJson(configuration, json);
		if (error.has_value()) {
			std::cout << error->what() << std::endl;
		}
		ASSERT_FALSE(error.has_value());
		ASSERT_EQ(configuration.getListenAddresses().size(), 2U);
		ASSERT_EQ(configuration.getListenAddresses().at(0), "127.0.0.1:80");
		ASSERT_EQ(configuration.getListenAddresses().at(1), "0.0.0.0:1080");
		ASSERT_EQ(configuration.getMaxInitialRequestBytes(), 524290U);
		ASSERT_EQ(configuration.getMaxInitialRequestPackets(), 513U);
		ASSERT_EQ(configuration.getRequestTimeout().count(), 60001U);
		ASSERT_EQ(configuration.getRequestQueueSize(), 101U);
		ASSERT_EQ(configuration.getRequestBodyQueueSize(), 51U);
	}
}

TEST(HttpServerConfiguration, dumpJson) {
	cpv::HttpServerConfiguration configuration;
	configuration.setListenAddresses({ "127.0.0.1:80", "0.0.0.0:1080" });
	cpv::Packet packet = cpv::serializeJson(configuration);
	ASSERT_EQ(packet.toString(),
		"{\"listenAddresses\":[\"127.0.0.1:80\",\"0.0.0.0:1080\"],"
		"\"maxInitialRequestBytes\":524288,\"maxInitialRequestPackets\":512,"
		"\"requestTimeout\":60000,\"requestQueueSize\":100,"
		"\"requestBodyQueueSize\":50}");
}

