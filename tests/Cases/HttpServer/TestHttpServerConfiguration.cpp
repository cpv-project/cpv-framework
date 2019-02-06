#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestHttpServerConfiguration, all) {
	cpv::HttpServerConfiguration configuration;
	ASSERT_TRUE(configuration.getListenAddresses().empty());
	ASSERT_EQ(configuration.getMaxInitialRequestBytes(), 524288U);
	ASSERT_EQ(configuration.getMaxInitialRequestPackets(), 512U);
	
	configuration.setListenAddresses({ "127.0.0.1:80", "0.0.0.0:1080" });
	configuration.setMaxInitialRequestBytes(524290);
	configuration.setMaxInitialRequestPackets(513);
	
	ASSERT_EQ(configuration.getListenAddresses().size(), 2U);
	ASSERT_EQ(configuration.getListenAddresses().at(0), "127.0.0.1:80");
	ASSERT_EQ(configuration.getListenAddresses().at(1), "0.0.0.0:1080");
	ASSERT_EQ(configuration.getMaxInitialRequestBytes(), 524290U);
	ASSERT_EQ(configuration.getMaxInitialRequestPackets(), 513U);
}

