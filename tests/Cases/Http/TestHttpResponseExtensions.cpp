#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestHttpResponseExtensions, setHeader_size_t) {
	cpv::HttpResponse response;
	cpv::extensions::setHeader(response, "Test-First", 123);
	cpv::extensions::setHeader(response, "Test-Second", 987654321);
	ASSERT_EQ(response.getHeaders().at("Test-First"), "123");
	ASSERT_EQ(response.getHeaders().at("Test-Second"), "987654321");
	ASSERT_FALSE(response.getUnderlyingBuffers().empty());
}

