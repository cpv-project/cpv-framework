#include <CPVFramework/Http/HttpRequestExtensions.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestHttpRequestExtensions, setHeader_size_t) {
	cpv::HttpRequest request;
	cpv::extensions::setHeader(request, "Test-First", 123);
	cpv::extensions::setHeader(request, "Test-Second", 987654321);
	ASSERT_EQ(request.getHeaders().at("Test-First"), "123");
	ASSERT_EQ(request.getHeaders().at("Test-Second"), "987654321");
	ASSERT_FALSE(request.getUnderlyingBuffers().empty());
}

