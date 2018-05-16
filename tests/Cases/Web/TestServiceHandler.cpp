#include <core/sleep.hh>
#include <CPVFramework/Web/ServiceHandler.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	class MyService {
	public:
		static void reset() { }
		static void freeResources() { }

		seastar::sstring replyText(cpv::httpd::const_req) {
			return "text";
		}

		seastar::future<cpv::Json> replyJson(cpv::httpd::const_req) {
			return seastar::sleep(std::chrono::milliseconds(1)).then([] {
				cpv::Json result;
				result["abc"] = 123;
				return result;
			});
		}

		seastar::future<> replyCustom(cpv::httpd::const_req, cpv::httpd::reply& reply) {
			return seastar::sleep(std::chrono::milliseconds(1)).then([&reply] {
				reply._content = "<html>custom</html>";
				reply.set_mime_type("text/html");
				reply.done();
			});
		}
	};
}

TEST_FUTURE(TestServiceHandler, text) {
	auto container = cpv::Container::create();
	container->add<MyService, MyService>();
	cpv::ServiceHandler<MyService, &MyService::replyText> handler(container);
	return handler.handle(
		"/reply_text",
		std::make_unique<cpv::httpd::request>(),
		std::make_unique<cpv::httpd::reply>()).then([] (auto reply) {
		ASSERT_EQ(reply->_content, "text");
		ASSERT_EQ(reply->_headers["Content-Type"], "text/plain");
	});
}

TEST_FUTURE(TestServiceHandler, json) {
	auto container = cpv::Container::create();
	container->add<MyService, MyService>();
	cpv::ServiceHandler<MyService, &MyService::replyJson> handler(container);
	return handler.handle(
		"/reply_json",
		std::make_unique<cpv::httpd::request>(),
		std::make_unique<cpv::httpd::reply>()).then([] (auto reply) {
		ASSERT_EQ(reply->_content, "{\"abc\":123}");
		ASSERT_EQ(reply->_headers["Content-Type"], "application/json");
	});
}

TEST_FUTURE(TestServiceHandler, custom) {
	auto container = cpv::Container::create();
	container->add<MyService, MyService>();
	cpv::ServiceHandler<MyService, &MyService::replyCustom> handler(container);
	return handler.handle(
		"/reply_custom",
		std::make_unique<cpv::httpd::request>(),
		std::make_unique<cpv::httpd::reply>()).then([] (auto reply) {
		ASSERT_EQ(reply->_content, "<html>custom</html>");
		ASSERT_EQ(reply->_headers["Content-Type"], "text/html");
	});
}

TEST_FUTURE(TestServiceHandler, serviceWithSeastarSharedPtr) {
	auto container = cpv::Container::create();
	container->add<seastar::shared_ptr<MyService>, seastar::shared_ptr<MyService>>();
	cpv::ServiceHandler<seastar::shared_ptr<MyService>, &MyService::replyText> handler(container);
	return handler.handle(
		"/reply_text",
		std::make_unique<cpv::httpd::request>(),
		std::make_unique<cpv::httpd::reply>()).then([] (auto reply) {
		ASSERT_EQ(reply->_content, "text");
		ASSERT_EQ(reply->_headers["Content-Type"], "text/plain");
	});
}

TEST_FUTURE(TestServiceHandler, serviceWithSeastarLwSharedPtr) {
	auto container = cpv::Container::create();
	container->add<seastar::lw_shared_ptr<MyService>, seastar::lw_shared_ptr<MyService>>();
	cpv::ServiceHandler<seastar::lw_shared_ptr<MyService>, &MyService::replyText> handler(container);
	return handler.handle(
		"/reply_text",
		std::make_unique<cpv::httpd::request>(),
		std::make_unique<cpv::httpd::reply>()).then([] (auto reply) {
		ASSERT_EQ(reply->_content, "text");
		ASSERT_EQ(reply->_headers["Content-Type"], "text/plain");
	});
}

TEST_FUTURE(TestServiceHandler, serviceWithStdUniquePtr) {
	auto container = cpv::Container::create();
	container->add<std::unique_ptr<MyService>, std::unique_ptr<MyService>>();
	cpv::ServiceHandler<std::unique_ptr<MyService>, &MyService::replyText> handler(container);
	return handler.handle(
		"/reply_text",
		std::make_unique<cpv::httpd::request>(),
		std::make_unique<cpv::httpd::reply>()).then([] (auto reply) {
		ASSERT_EQ(reply->_content, "text");
		ASSERT_EQ(reply->_headers["Content-Type"], "text/plain");
	});
}

TEST_FUTURE(TestServiceHandler, serviceWithStdSharedPtr) {
	auto container = cpv::Container::create();
	container->add<std::shared_ptr<MyService>, std::shared_ptr<MyService>>();
	cpv::ServiceHandler<std::shared_ptr<MyService>, &MyService::replyText> handler(container);
	return handler.handle(
		"/reply_text",
		std::make_unique<cpv::httpd::request>(),
		std::make_unique<cpv::httpd::reply>()).then([] (auto reply) {
		ASSERT_EQ(reply->_content, "text");
		ASSERT_EQ(reply->_headers["Content-Type"], "text/plain");
	});
}

TEST_FUTURE(TestServiceHandler, serviceWithObject) {
	auto container = cpv::Container::create();
	container->add<cpv::Object<MyService>, cpv::Object<MyService>>();
	cpv::ServiceHandler<cpv::Object<MyService>, &MyService::replyText> handler(container);
	return handler.handle(
		"/reply_text",
		std::make_unique<cpv::httpd::request>(),
		std::make_unique<cpv::httpd::reply>()).then([] (auto reply) {
		ASSERT_EQ(reply->_content, "text");
		ASSERT_EQ(reply->_headers["Content-Type"], "text/plain");
	});
}

