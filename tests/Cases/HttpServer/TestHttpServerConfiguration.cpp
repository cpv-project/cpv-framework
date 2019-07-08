#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestHttpServerConfiguration, all) {
	cpv::HttpServerConfiguration configuration;
	ASSERT_TRUE(configuration.getListenAddresses().empty());
	ASSERT_EQ(configuration.getMaxInitialRequestBytes(), 524288U);
	ASSERT_EQ(configuration.getMaxInitialRequestPackets(), 512U);
	ASSERT_EQ(configuration.getRequestTimeout().count(), 30000U);
	ASSERT_EQ(configuration.getRequestQueueSize(), 100U);
	ASSERT_EQ(configuration.getRequestBodyQueueSize(), 50U);
	
	configuration.setListenAddresses({ "127.0.0.1:80", "0.0.0.0:1080" });
	configuration.setMaxInitialRequestBytes(524290);
	configuration.setMaxInitialRequestPackets(513);
	configuration.setRequestTimeout(std::chrono::milliseconds(30001));
	configuration.setRequestQueueSize(101);
	configuration.setRequestBodyQueueSize(51);
	
	ASSERT_EQ(configuration.getListenAddresses().size(), 2U);
	ASSERT_EQ(configuration.getListenAddresses().at(0), "127.0.0.1:80");
	ASSERT_EQ(configuration.getListenAddresses().at(1), "0.0.0.0:1080");
	ASSERT_EQ(configuration.getMaxInitialRequestBytes(), 524290U);
	ASSERT_EQ(configuration.getMaxInitialRequestPackets(), 513U);
	ASSERT_EQ(configuration.getRequestTimeout().count(), 30001U);
	ASSERT_EQ(configuration.getRequestQueueSize(), 101U);
	ASSERT_EQ(configuration.getRequestBodyQueueSize(), 51U);
}

