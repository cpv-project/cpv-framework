#include <CPVFramework/Module/PrometheusModule.hpp>
#include <CPVFramework/Utility/JsonUtils.hpp>
#include <CPVFramework/Http/Handler.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestPrometheusModule, metrics) {
	auto server = seastar::make_shared<cpv::httpd::http_server>("test-prometheus-module");
	auto container = cpv::Container::create();
	cpv::Json configuration = cpv::Json::parse("{}");
	cpv::PrometheusModule module(seastar::make_shared<cpv::Json>(configuration));
	module.registerRoutes(container, *server);
	auto handler = server->_routes.get_exact_match(cpv::httpd::operation_type::GET, "/metrics");
	ASSERT_NE(handler, nullptr);
}

