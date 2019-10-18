#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestRoutingHandler.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace {
	template <std::size_t Number>
	class MyHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpContext& context,
			const cpv::HttpServerRequestHandlerIterator&) const {
			auto& response = context.getResponse();
			auto buffer = cpv::convertIntToBuffer(Number);
			return cpv::extensions::reply(response, std::move(buffer));
		}
	};
}

TEST(HttpServerRequestRoutingHandler, basic) {
	auto getRoot = seastar::make_shared<MyHandler<0>>();
	auto postRoot = seastar::make_shared<MyHandler<0>>();
	auto getTest = seastar::make_shared<MyHandler<0>>();
	auto postTest = seastar::make_shared<MyHandler<0>>();
	auto getUserInfo = seastar::make_shared<MyHandler<0>>();
	auto patchUserInfo = seastar::make_shared<MyHandler<0>>();
	auto getUserList = seastar::make_shared<MyHandler<0>>();
	auto getStatic = seastar::make_shared<MyHandler<0>>();
	auto getRemoveMap = seastar::make_shared<MyHandler<0>>();
	auto getRemoveTree = seastar::make_shared<MyHandler<0>>();

	cpv::HttpServerRequestRoutingHandler handler;
	handler.route(cpv::constants::GET, "/", getRoot);
	handler.route(cpv::constants::POST, "/", postRoot);
	handler.route(cpv::constants::GET, "/test", getTest);
	handler.route(cpv::constants::POST, "/test", postTest);
	handler.route(cpv::constants::GET, "/api/v1/user/*/info", getUserInfo);
	handler.route(cpv::constants::PATCH, "/api/v1/user/*/info", patchUserInfo);
	handler.route(cpv::constants::GET, "/api/v1/user/list", getUserList);
	handler.route(cpv::constants::GET, "/static/**", getStatic);
	handler.route(cpv::constants::GET, "/remove/map", getRemoveMap);
	handler.route(cpv::constants::GET, "/remove/tree/*", getRemoveTree);

	handler.removeRoute(cpv::constants::GET, "/remove/map");
	handler.removeRoute(cpv::constants::GET, "/remove/tree/*");
	handler.removeRoute(cpv::constants::GET, "/remove/non-exists/*");

	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/").get(), getRoot.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::POST, "/").get(), postRoot.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/test").get(), getTest.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::POST, "/test").get(), postTest.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::POST, "/test?id=123").get(), postTest.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/api/v1/user/1/info").get(), getUserInfo.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/api/v1/user/123/info").get(), getUserInfo.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/api/v1/user/321/info/").get(), getUserInfo.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/api/v1/user/321/info/?id=321").get(), getUserInfo.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::PATCH, "/api/v1/user/1/info").get(), patchUserInfo.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::PATCH, "/api/v1/user/123/info").get(), patchUserInfo.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::PATCH, "/api/v1/user/123/info/").get(), patchUserInfo.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/api/v1/user/info").get(), nullptr);
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/api/v1/user/123/321/info").get(), nullptr);
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/api/v1/user/list").get(), getUserList.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/static/1.txt").get(), getStatic.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/static/pic/100.jpg").get(), getStatic.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/static/uploads/a/abc.jpg").get(), getStatic.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/static/uploads/a/abc.jpg?mtime=123").get(), getStatic.get());
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/static").get(), nullptr);
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/remove/map").get(), nullptr);
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/remove/tree/123").get(), nullptr);
	ASSERT_EQ(handler.getRoute(cpv::constants::GET, "/non-exist").get(), nullptr);
}

TEST_FUTURE(HttpServerRequestRoutingHandler, handle) {
	return seastar::do_with(
		cpv::HttpServerRequestHandlerCollection(),
		cpv::HttpContext(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& handlers, auto& context, auto& str) {
		auto handler = seastar::make_shared<cpv::HttpServerRequestRoutingHandler>();
		handler->route(cpv::constants::GET, "/", seastar::make_shared<MyHandler<0>>());
		handler->route(cpv::constants::POST, "/", seastar::make_shared<MyHandler<1>>());
		handler->route(cpv::constants::GET, "/test", seastar::make_shared<MyHandler<2>>());
		handler->route(cpv::constants::POST, "/test", seastar::make_shared<MyHandler<3>>());
		handler->route(cpv::constants::GET, "/api/v1/user/*/info", seastar::make_shared<MyHandler<4>>());
		handler->route(cpv::constants::PATCH, "/api/v1/user/*/info", seastar::make_shared<MyHandler<5>>());
		handler->route(cpv::constants::GET, "/api/v1/user/list", seastar::make_shared<MyHandler<6>>());
		handler->route(cpv::constants::GET, "/static/**", seastar::make_shared<MyHandler<7>>());
		handler->route(cpv::constants::GET, "/remove/map", seastar::make_shared<MyHandler<8>>());
		handler->route(cpv::constants::GET, "/remove/tree/*", seastar::make_shared<MyHandler<9>>());

		handler->removeRoute(cpv::constants::GET, "/remove/map");
		handler->removeRoute(cpv::constants::GET, "/remove/tree/*");
		handler->removeRoute(cpv::constants::GET, "/remove/non-exists/*");

		handlers.emplace_back(handler);
		handlers.emplace_back(seastar::make_shared<cpv::HttpServerRequest404Handler>());

		context.getResponse().setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());

		auto test = [&handlers, &context, &str] (std::string_view method, std::string_view path) {
			context.getRequest().setMethod(method);
			context.getRequest().setUrl(path);
			return handlers.at(0)->handle(context, handlers.begin() + 1).then([&str] {
				str->append(",");
			});
		};

		return test(cpv::constants::GET, "/")
			.then([test] { return test(cpv::constants::POST, "/"); })
			.then([test] { return test(cpv::constants::GET, "/test"); })
			.then([test] { return test(cpv::constants::POST, "/test?id=123"); })
			.then([test] { return test(cpv::constants::GET, "/api/v1/user/1/info"); })
			.then([test] { return test(cpv::constants::GET, "/api/v1/user/123/info"); })
			.then([test] { return test(cpv::constants::GET, "/api/v1/user/321/info/?id=321"); })
			.then([test] { return test(cpv::constants::PATCH, "/api/v1/user/1/info"); })
			.then([test] { return test(cpv::constants::PATCH, "/api/v1/user/123/info"); })
			.then([test] { return test(cpv::constants::PATCH, "/api/v1/user/123/info/"); })
			.then([test] { return test(cpv::constants::GET, "/api/v1/user/info"); })
			.then([test] { return test(cpv::constants::GET, "/api/v1/user/123/321/info"); })
			.then([test] { return test(cpv::constants::GET, "/api/v1/user/list"); })
			.then([test] { return test(cpv::constants::GET, "/static/1.txt"); })
			.then([test] { return test(cpv::constants::GET, "/static/pic/100.jpg"); })
			.then([test] { return test(cpv::constants::GET, "/static/uploads/a/abc.jpg?mtime=123"); })
			.then([test] { return test(cpv::constants::GET, "/static"); })
			.then([test] { return test(cpv::constants::GET, "/remove/map"); })
			.then([test] { return test(cpv::constants::GET, "/remove/tree/123"); })
			.then([test] { return test(cpv::constants::GET, "/non-exist"); })
			.then([&str] {
				ASSERT_EQ(*str, "0,1,2,3,4,4,4,5,5,5,Not Found,Not Found,6,7,7,7,Not Found,Not Found,Not Found,Not Found,");
			});
	});
}

TEST_FUTURE(HttpServerRequestRoutingHandler, routeFunc) {
	return seastar::do_with(
		cpv::HttpServerRequestHandlerCollection(),
		cpv::HttpContext(),
		seastar::make_lw_shared<std::string>(),
		[] (auto& handlers, auto& context, auto& str) {
		auto handler = seastar::make_shared<cpv::HttpServerRequestRoutingHandler>();
		handler->route(cpv::constants::GET, "/func", [](auto& context) {
			auto& response = context.getResponse();
			return cpv::extensions::reply(response, "test function handler");
		});
		handler->route(cpv::constants::GET, "/get/*/details", std::make_tuple(1, "keyword"),
			[](auto& context, auto id, auto keyword) {
				auto& response = context.getResponse();
				return cpv::extensions::reply(response, cpv::joinString("-", id, keyword));
			});

		handlers.emplace_back(handler);
		handlers.emplace_back(seastar::make_shared<cpv::HttpServerRequest404Handler>());

		context.getResponse().setBodyStream(
			cpv::makeReusable<cpv::StringOutputStream>(str).template cast<cpv::OutputStreamBase>());

		auto test = [&handlers, &context, &str] (std::string_view method, std::string_view path) {
			context.getRequest().setMethod(method);
			context.getRequest().setUrl(path);
			return handlers.at(0)->handle(context, handlers.begin() + 1).then([&str] {
				str->append(",");
			});
		};

		return test(cpv::constants::GET, "/func")
			.then([test] { return test(cpv::constants::GET, "/get/123/details?keyword=abc"); })
			.then([&str] {
				ASSERT_EQ(*str, "test function handler,123-abc,");
			});
	});
}

