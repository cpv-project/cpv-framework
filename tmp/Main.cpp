#include <signal.h>
#include <atomic>
#include <iostream>
#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/HttpServer/HttpServer.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/ConstantStrings.hpp>

namespace {
	std::atomic_bool StopFlag(false);
	
	class CustomHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpContext& context,
			const cpv::HttpServerRequestHandlerIterator&) const override {
			using namespace cpv;
			auto& request = context.request;
			auto& response = context.response;
			Packet p;
			p.append("request method: ").append(request.getMethod()).append("\r\n")
				.append("request url: ").append(request.getUrl()).append("\r\n")
				.append("request version: ").append(request.getVersion()).append("\r\n")
				.append("request headers:\r\n");
			request.getHeaders().foreach([&p] (const auto& key, const auto& value) {
				p.append("  ").append(key).append(": ").append(value).append("\r\n");
			});
			response.setStatusCode(constants::_200);
			response.setStatusMessage(constants::OK);
			response.setHeader(constants::ContentType, constants::TextPlainUtf8);
			response.setHeader(constants::ContentLength, p.size());
			return extensions::writeAll(response.getBodyStream(), std::move(p));
		}
	};

	class HelloHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpContext& context,
			const cpv::HttpServerRequestHandlerIterator&) const override {
			using namespace cpv;
			auto& response = context.response;
			response.setStatusCode(constants::_200);
			response.setStatusMessage(constants::OK);
			response.setHeader(constants::ContentType, constants::TextPlainUtf8);
			response.setHeader(constants::ContentLength, 12);
			return extensions::writeAll(response.getBodyStream(), "Hello World!");
		}
	};
	
	seastar::future<> service_loop() {
		cpv::HttpServerConfiguration configuration;
		configuration.setListenAddresses({ "0.0.0.0:8000", "127.0.0.1:8001" });

		auto logger = cpv::Logger::createConsole(cpv::LogLevel::Notice);

		cpv::HttpServerRequestHandlerCollection handlers;
		handlers.emplace_back(seastar::make_shared<cpv::HttpServerRequest500Handler>(logger));
		// handlers.emplace_back(seastar::make_shared<CustomHandler>());
		handlers.emplace_back(seastar::make_shared<HelloHandler>());
		handlers.emplace_back(seastar::make_shared<cpv::HttpServerRequest404Handler>());

		cpv::Container container;
		container.add(configuration);
		container.add(logger);
		for (auto& handler : handlers) {
			container.add(handler);
		}

		cpv::HttpServer server(container);
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
		}).then([] {
			// wait for internal cleanup to make leak sanitizer happy
			return seastar::sleep(std::chrono::seconds(1));
		});
	});
	return 0;
}

