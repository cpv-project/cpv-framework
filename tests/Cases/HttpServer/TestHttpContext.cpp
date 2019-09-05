#include <CPVFramework/HttpServer/HttpContext.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

TEST(TestHttpContext, all) {
	{
		cpv::HttpContext context;
		ASSERT_TRUE(context.getRequest().getMethod().empty());
		ASSERT_TRUE(context.getResponse().getStatusCode().empty());
		ASSERT_EQ(cpv::joinString("", context.getClientAddress()), "0.0.0.0:0");
		std::vector<int> ints;
		context.getManyServices(ints);
		ASSERT_TRUE(ints.empty());
	}
	{
		cpv::HttpRequest request;
		request.setMethod(cpv::constants::POST);
		cpv::HttpResponse response;
		response.setStatusCode(cpv::constants::_200);
		seastar::socket_address clientAddress(seastar::make_ipv4_address(0x7f000001, 10000));
		cpv::Container container;
		container.add<seastar::shared_ptr<int>>([] {
			return seastar::make_shared<int>(123);
		}, cpv::ServiceLifetime::StoragePersistent);
		
		cpv::HttpContext context(nullptr);
		context.setRequestResponse(std::move(request), std::move(response));
		context.setClientAddress(std::move(clientAddress));
		context.setContainer(std::move(container));
		
		ASSERT_EQ(context.getRequest().getMethod(), cpv::constants::POST);
		ASSERT_EQ(context.getResponse().getStatusCode(), cpv::constants::_200);
		ASSERT_EQ(cpv::joinString("", context.getClientAddress()), "127.0.0.1:10000");
		auto a = context.getService<seastar::shared_ptr<int>>();
		auto b = context.getService<seastar::shared_ptr<int>>();
		std::vector<seastar::shared_ptr<int>> c;
		context.getManyServices(c);
		ASSERT_EQ(a.get(), b.get());
		ASSERT_EQ(c.size(), 1U);
		ASSERT_EQ(a.get(), c.at(0).get());

		context.clearServiceStorage();
		auto d = context.getService<seastar::shared_ptr<int>>();
		ASSERT_NE(a.get(), d.get());
	}
}

