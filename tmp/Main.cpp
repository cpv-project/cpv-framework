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
#include <CPVFramework/Utility/PacketUtils.hpp>

namespace {
	std::atomic_bool StopFlag(false);
	
	class CustomHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpRequest& request,
			cpv::HttpResponse& response,
			const cpv::HttpServerRequestHandlerIterator&) const override {
			using namespace cpv;
			seastar::net::packet p;
			p << "request method: " << request.getMethod() << "\r\n";
			p << "request url: " << request.getUrl() << "\r\n";
			p << "request version: " << request.getVersion() << "\r\n";
			p << "request headers:\r\n";
			p << "";
			for (auto& pair : request.getHeaders()) {
				p << "  " << pair.first << ": " << pair.second << "\r\n";
			}
			response.setStatusCode(constants::_200);
			response.setStatusMessage(constants::OK);
			response.setHeader(constants::ContentType, constants::TextPlainUtf8);
			if (p.len() < constants::Integers.size()) {
				response.setHeader(constants::ContentLength, constants::Integers.at(p.len()));
			} else {
				auto buf = convertIntToBuffer(p.len());
				response.setHeader(constants::ContentLength, std::string_view(buf.get(), buf.size()));
				response.addUnderlyingBuffer(std::move(buf));
			}
			return extensions::writeAll(response.getBodyStream(), std::move(p));
		}
	};

	class HelloHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpRequest&,
			cpv::HttpResponse& response,
			const cpv::HttpServerRequestHandlerIterator&) const override {
			using namespace cpv;
			response.setStatusCode(constants::_200);
			response.setStatusMessage(constants::OK);
			response.setHeader(constants::ContentType, constants::TextPlainUtf8);
			response.setHeader(constants::ContentLength, constants::Integers.at(12));
			return cpv::extensions::writeAll(response.getBodyStream(), "Hello World!");
		}
	};
	
	seastar::future<> service_loop() {
		cpv::HttpServerConfiguration configuration;
		configuration.setListenAddresses({ "0.0.0.0:8000", "127.0.0.1:8001" });
		auto logger = cpv::Logger::createConsole(cpv::LogLevel::Notice);
		std::vector<std::unique_ptr<cpv::HttpServerRequestHandlerBase>> handlers;
		handlers.emplace_back(std::make_unique<cpv::HttpServerRequest500Handler>(logger));
		// handlers.emplace_back(std::make_unique<CustomHandler>());
		handlers.emplace_back(std::make_unique<HelloHandler>());
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
		}).then([] {
			// wait for internal cleanup to make leak sanitizer happy
			return seastar::sleep(std::chrono::seconds(1));
		});
	});
	return 0;
}

