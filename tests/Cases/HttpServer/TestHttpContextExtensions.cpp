#include <CPVFramework/HttpServer/HttpContextExtensions.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(HttpContextExtensions, getParameter_PathFragment) {
	using namespace cpv::extensions::http_context_parameters;
	cpv::HttpContext context;
	context.getRequest().setUrl("/test/abc/123");
	ASSERT_EQ(cpv::extensions::getParameter(context, PathFragment(0)), "test");
	ASSERT_EQ(cpv::extensions::getParameter(context, PathFragment(1)), "abc");
	ASSERT_EQ(cpv::extensions::getParameter(context, PathFragment(2)), "123");
	ASSERT_EQ(cpv::extensions::getParameter(context, PathFragment(3)), "");
}

TEST(HttpContextExtensions, getParameter_Query) {
	using namespace cpv::extensions::http_context_parameters;
	cpv::HttpContext context;
	context.getRequest().setUrl("/test?key=123&value=321");
	ASSERT_EQ(cpv::extensions::getParameter(context, Query("key")), "123");
	ASSERT_EQ(cpv::extensions::getParameter(context, Query("value")), "321");
	ASSERT_EQ(cpv::extensions::getParameter(context, Query("non-exists")), "");
}

