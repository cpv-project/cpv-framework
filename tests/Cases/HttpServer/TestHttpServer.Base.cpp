#include <seastar/core/future-util.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/HttpServer/HttpServer.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Utility/PacketUtils.hpp>
#include "./TestHttpServer.Base.hpp"

namespace cpv::gtest {
	/** Generic test runner for http server */
	seastar::future<> runHttpServerTest(HttpServerTestFunctions&& testFunctions) {
		return seastar::do_with(
			std::move(testFunctions),
			std::make_unique<std::atomic_bool>(false),
			std::exception_ptr(),
			[] (auto& testFunctions, auto& stopFlag, auto& error) {
			return seastar::parallel_for_each(boost::irange<unsigned>(0, seastar::smp::count),
				[&testFunctions, &stopFlag, &error] (unsigned c) {
				return seastar::smp::submit_to(c, [c, &testFunctions, &stopFlag, &error] {
					// make configuration
					cpv::HttpServerConfiguration configuration;
					configuration.setListenAddresses({
						cpv::joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
						cpv::joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
					});
					if (testFunctions.updateConfiguration != nullptr) {
						testFunctions.updateConfiguration(configuration);
					}
					// make logger
					auto logger = cpv::Logger::createConsole(cpv::LogLevel::Debug);
					// make handlers
					HttpServerRequestHandlerCollection handlers;
					handlers.emplace_back(std::make_unique<cpv::HttpServerRequest500Handler>(logger));
					if (testFunctions.makeHandlers != nullptr) {
						auto customHandlers = testFunctions.makeHandlers();
						std::move(customHandlers.begin(), customHandlers.end(), std::back_inserter(handlers));
					}
					handlers.emplace_back(std::make_unique<cpv::HttpServerRequest404Handler>());
					// start http server
					cpv::HttpServer server(configuration, logger, std::move(handlers));
					return seastar::do_with(std::move(server),
						[c, &testFunctions, &stopFlag, &error](auto& server) {
						return server.start().then([c, &testFunctions, &stopFlag, &error] {
							// target function will run on core 0
							if (c == 0) {
								return testFunctions.execute().then([&stopFlag] {
									stopFlag->store(true);
								}).handle_exception([&stopFlag, &error] (std::exception_ptr ex) {
									error = std::move(ex);
									stopFlag->store(true);
								});
							} else {
								return seastar::do_until(
									[&stopFlag] { return stopFlag->load(); },
									[] { return seastar::sleep(std::chrono::milliseconds(50)); });
							}
						}).then([&server] {
							return server.stop();
						}).then([&error] {
							if (error == nullptr) {
								return seastar::make_ready_future<>();
							} else {
								return seastar::make_exception_future<>(std::move(error));
							}
						});
					});
				});
			});
		});
	}
	
	namespace {
		// Provide a fixed date value to make the response content always same
		const std::string_view PesudoDate("Thu, 01 Jan 1970 00:00:00 GMT");
	}
	
	/** Reply request url and header values in response body */
	seastar::future<> HttpCheckHeadersHandler::handle(
		cpv::HttpRequest& request,
		cpv::HttpResponse& response,
		const cpv::HttpServerRequestHandlerIterator&) const {
		using namespace cpv;
		seastar::net::packet p;
		p << "request method: " << request.getMethod() << "\r\n";
		p << "request url: " << request.getUrl() << "\r\n";
		p << "request version: " << request.getVersion() << "\r\n";
		p << "request headers:\r\n";
		auto& headers = request.getHeaders();
		std::vector<std::string_view> headerFields;
		std::transform(headers.begin(), headers.end(),
			std::back_inserter(headerFields), [] (auto& pair) { return pair.first; });
		std::sort(headerFields.begin(), headerFields.end());
		for (auto& headerField : headerFields) {
			p << "  " << headerField << ": " << headers.at(headerField) << "\r\n";
		}
		response.setStatusCode(constants::_200);
		response.setStatusMessage(constants::OK);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::Date, PesudoDate);
		extensions::setHeader(response, constants::ContentLength, p.len());
		return extensions::writeAll(response.getBodyStream(), std::move(p));
	}
	
	/** Reply request body in response body */
	seastar::future<> HttpCheckBodyHandler::handle(
		cpv::HttpRequest& request,
		cpv::HttpResponse& response,
		const cpv::HttpServerRequestHandlerIterator&) const {
		return extensions::readAll(request.getBodyStream()).then([&request, &response] (auto str) {
			response.setStatusCode(constants::_200);
			response.setStatusMessage(constants::OK);
			response.setHeader(constants::ContentType, request.getHeaders().at(constants::ContentType));
			response.setHeader(constants::Date, PesudoDate);
			extensions::setHeader(response, constants::ContentLength, str.size());
			return extensions::writeAll(response.getBodyStream(), std::move(str));
		});
	}
	
	/** Reply response with body but without content length header */
	seastar::future<> HttpLengthNotFixedHandler::handle(
		cpv::HttpRequest&,
		cpv::HttpResponse& response,
		const cpv::HttpServerRequestHandlerIterator&) const {
		response.setStatusCode(constants::_200);
		response.setStatusMessage(constants::OK);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::Date, PesudoDate);
		return extensions::writeAll(response.getBodyStream(), "Length Not Fixed");
	}
	
	/** Reply response with body but size not matched to content length header */
	seastar::future<> HttpWrittenSizeNotMatchedHandler::handle(
		cpv::HttpRequest&,
		cpv::HttpResponse& response,
		const cpv::HttpServerRequestHandlerIterator&) const {
		response.setStatusCode(constants::_200);
		response.setStatusMessage(constants::OK);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::Date, PesudoDate);
		extensions::setHeader(response, constants::ContentLength, 1);
		return extensions::writeAll(response.getBodyStream(), "Written Size Not Matched");
	}
}

