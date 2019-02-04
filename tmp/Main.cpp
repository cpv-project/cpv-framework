#include <signal.h>
#include <atomic>
#include <iostream>
#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <CPVFramework/HttpServer/HttpServer.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>

namespace {
	std::atomic_bool StopFlag(false);
	
	seastar::future<> service_loop() {
		cpv::HttpServerConfiguration configuration;
		auto logger = cpv::Logger::createConsole(cpv::LogLevel::Debug);
		std::vector<std::unique_ptr<cpv::HttpServerRequestHandlerBase>> handlers;
		handlers.emplace_back(std::make_unique<cpv::HttpServerRequest500Handler>(logger));
		handlers.emplace_back(std::make_unique<cpv::HttpServerRequest404Handler>());
		cpv::HttpServer server(configuration, logger, std::move(handlers));
		return seastar::do_with(std::move(server), [](auto& server) {
			return server.start().then([] {
				return seastar::do_until(
					[] { return StopFlag.load(); },
					[] { return seastar::sleep(std::chrono::seconds(1)); });
			}).then([&server] {
				return server.stop();
			});
		});
	}
}

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		seastar::engine().at_exit([] {
			StopFlag.store(true);
			return seastar::make_ready_future();
		});
		return seastar::parallel_for_each(boost::irange<unsigned>(0, seastar::smp::count),
			[] (unsigned c) {
				return seastar::smp::submit_to(c, service_loop);
		});
	});
	return 0;
}

