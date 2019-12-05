#include <CPVFramework/Utility/NetworkUtils.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Exceptions/NotImplementedException.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(NetworkUtils, parseListenAddress) {
	ASSERT_EQ(cpv::joinString("", cpv::parseListenAddress("192.168.0.1:1000")), "192.168.0.1:1000");
	ASSERT_EQ(cpv::joinString("", cpv::parseListenAddress("127.0.0.1:65535")), "127.0.0.1:65535");
	ASSERT_EQ(cpv::joinString("", cpv::parseListenAddress(":1")), "0.0.0.0:1");
	ASSERT_THROWS_CONTAINS(
		cpv::FormatException,
		cpv::parseListenAddress("127.0.0.0.1"),
		"no ':' in listen address: 127.0.0.0.1");
	ASSERT_THROWS_CONTAINS(
		cpv::FormatException,
		cpv::parseListenAddress("127.0.0.256:1"),
		"invalid listen ip address: 127.0.0.256:1");
	ASSERT_THROWS_CONTAINS(
		cpv::FormatException,
		cpv::parseListenAddress("127.0.0.1:0"),
		"invalid listen port: 127.0.0.1:0");
	ASSERT_THROWS_CONTAINS(
		cpv::FormatException,
		cpv::parseListenAddress("127.0.0.1:65536"),
		"invalid listen port: 127.0.0.1:65536");
	ASSERT_THROWS_CONTAINS(
		cpv::FormatException,
		cpv::parseListenAddress("127.0.0.1:a"),
		"invalid listen port: 127.0.0.1:a");
}

