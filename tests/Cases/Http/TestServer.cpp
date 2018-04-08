#include <CPVFramework/Http/Server.hpp>
#include <CPVFramework/Http/Route.hpp>
#include <CPVFramework/Http/Handler.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST_FUTURE(TestHttpServer, simple) {
	auto server = seastar::make_shared<cpv::httpd::http_server_control>();
	return server->start().then([server] {
		return server->set_routes([] (cpv::httpd::routes& r) {
			r.put(
				cpv::httpd::operation_type::GET,
				"/",
				new cpv::httpd::function_handler([] (cpv::httpd::const_req req) {
					return "hello world";
				}));
		});
	}).then([server] {
		return server->listen({ HTTPD_LISTEN_IP, HTTPD_LISTEN_PORT });
	}).finally([server] {
		return server->stop();
	}).finally([server] { });
}

