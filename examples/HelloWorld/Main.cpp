#include <seastar/core/app-template.hh>
#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Application/Modules/LoggingModule.hpp>
#include <CPVFramework/Application/Modules/HttpServerModule.hpp>
#include <CPVFramework/Application/Modules/HttpServerRoutingModule.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		cpv::Application application;
		application.add<cpv::LoggingModule>();
		application.add<cpv::HttpServerModule>([] (auto& module) {
			module.getConfig().setListenAddresses({ "0.0.0.0:8000", "127.0.0.1:8001" });
		});
		application.add<cpv::HttpServerRoutingModule>([] (auto& module) {
			module.route(cpv::constants::GET, "/", [] (auto& context) {
				return cpv::extensions::reply(context.getResponse(), "Hello World!");
			});
		});
		return application.run_forever();
	});
	return 0;
}

