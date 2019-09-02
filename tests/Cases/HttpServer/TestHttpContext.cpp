#include <CPVFramework/HttpServer/HttpContext.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestHttpContext, all) {
	{
		cpv::HttpContext context;
		ASSERT_TRUE(context.request.getMethod().empty());
		ASSERT_TRUE(context.response.getStatusCode().empty());
		ASSERT_EQ(cpv::joinString("", context.clientAddress), "0.0.0.0:0");
		std::vector<int> ints;
		context.container.getMany(ints);
		ASSERT_TRUE(ints.empty());
		ASSERT_FALSE(context.serviceStorage.get(0).has_value());
	}
	{
		cpv::Container container;
		cpv::HttpContext context(container);
		container.add<int>(123);
		ASSERT_EQ(cpv::joinString("", context.clientAddress), "0.0.0.0:0");
		ASSERT_EQ(context.container.get<int>(), 123);
	}
	{
		cpv::Container container;
		cpv::HttpContext context(container,
			seastar::make_ipv4_address(0x7f000001, 10000));
		container.add<int>(123);
		ASSERT_EQ(cpv::joinString("", context.clientAddress), "127.0.0.1:10000");
		ASSERT_EQ(context.container.get<int>(), 123);
	}
}

