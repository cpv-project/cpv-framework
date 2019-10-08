#include <CPVFramework/Http/HttpRequestExtensions.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestHttpRequestExtensions, getParameter_size_t) {
	cpv::HttpRequest request;
	request.setUrl("/test/abc/123");
	ASSERT_EQ(cpv::extensions::getParameter(request, 0), "test");
	ASSERT_EQ(cpv::extensions::getParameter(request, 1), "abc");
	ASSERT_EQ(cpv::extensions::getParameter(request, 2), "123");
	ASSERT_EQ(cpv::extensions::getParameter(request, 3), "");
}

TEST(TestHttpRequestExtensions, getParameter_string_view) {
	cpv::HttpRequest request;
	request.setUrl("/test?key=123&value=321");
	ASSERT_EQ(cpv::extensions::getParameter(request, "key"), "123");
	ASSERT_EQ(cpv::extensions::getParameter(request, "value"), "321");
	ASSERT_EQ(cpv::extensions::getParameter(request, "non-exists"), "");
}

