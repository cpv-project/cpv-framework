#include <seastar/core/future-util.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Application/Modules/LoggingModule.hpp>
#include <CPVFramework/Application/Modules/HttpServerModule.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Stream/InputStreamExtensions.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include "./TestHttpServer.Base.hpp"

namespace cpv::gtest {
	/** Generic test runner for http server */
	seastar::future<> runHttpServerTest(HttpServerTestFunctions&& testFunctions) {
		return seastar::do_with(
			std::move(testFunctions),
			std::exception_ptr(),
			[] (auto& testFunctions, auto& error) {
			Application application;
			application.add<LoggingModule>([] (auto& module) {
				module.setLogger(Logger::createNoop());
			});
			application.add<HttpServerModule>([&testFunctions] (auto& module) {
				auto& config = module.getConfig();
				config.setListenAddresses({
					joinString("", HTTP_SERVER_1_IP, ":", HTTP_SERVER_1_PORT),
					joinString("", HTTP_SERVER_2_IP, ":", HTTP_SERVER_2_PORT),
				});
				if (testFunctions.updateConfiguration != nullptr) {
					testFunctions.updateConfiguration(config);
				}
				if (testFunctions.makeHandlers != nullptr) {
					auto customHandlers = testFunctions.makeHandlers();
					for (auto& handler : customHandlers) {
						module.addCustomHandler(handler);
					}
				}
			});
			return application.start().then([&testFunctions, &error] {
				return testFunctions.execute()
					.handle_exception([&error] (std::exception_ptr ex) {
						error = std::move(ex);
					});
			}).then([application] () mutable {
				return application.stop();
			}).then([&error] {
				if (error == nullptr) {
					return seastar::make_ready_future<>();
				} else {
					return seastar::make_exception_future<>(std::move(error));
				}
			});
		});
	}
	
	namespace {
		// Provide a fixed date value to make the response content always same
		static const constexpr char PesudoDate[] = "Thu, 01 Jan 1970 00:00:00 GMT";
	}
	
	/** Reply request url and header values in response body */
	seastar::future<> HttpCheckHeadersHandler::handle(
		HttpContext& context,
		HttpServerRequestHandlerIterator) const {
		using namespace cpv;
		auto& request = context.getRequest();
		auto& response = context.getResponse();
		Packet p;
		p.append("request method: ").append(request.getMethod().share()).append("\r\n")
			.append("request url: ").append(request.getUrl().share()).append("\r\n")
			.append("request version: ").append(request.getVersion().share()).append("\r\n")
			.append("request headers:\r\n");
		request.getHeaders().foreach([&p] (const cpv::SharedString& key, const cpv::SharedString& value) {
			p.append("  ").append(key.share()).append(": ").append(value.share()).append("\r\n");
		});
		response.setHeader(constants::Date, PesudoDate);
		return extensions::reply(response, std::move(p));
	}
	
	/** Reply request body in response body */
	seastar::future<> HttpCheckBodyHandler::handle(
		HttpContext& context,
		HttpServerRequestHandlerIterator) const {
		auto& request = context.getRequest();
		auto& response = context.getResponse();
		return extensions::readAll(request.getBodyStream()).then([&request, &response] (auto str) {
			response.setHeader(constants::Date, PesudoDate);
			return extensions::reply(response, std::move(str),
				request.getHeaders().getHeader(constants::ContentType));
		});
	}
	
	/** Reply response with body but without content length header */
	seastar::future<> HttpLengthNotFixedHandler::handle(
		HttpContext& context,
		HttpServerRequestHandlerIterator) const {
		auto& response = context.getResponse();
		response.setStatusCode(constants::_200);
		response.setStatusMessage(constants::OK);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::Date, PesudoDate);
		return extensions::writeAll(response.getBodyStream(), "Length Not Fixed");
	}
	
	/** Reply response with body but size not matched to content length header */
	seastar::future<> HttpWrittenSizeNotMatchedHandler::handle(
		HttpContext& context,
		HttpServerRequestHandlerIterator) const {
		auto& response = context.getResponse();
		response.setStatusCode(constants::_200);
		response.setStatusMessage(constants::OK);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::Date, PesudoDate);
		response.setHeader(constants::ContentLength, SharedString::fromInt(1));
		return extensions::writeAll(response.getBodyStream(), "Written Size Not Matched");
	}
}

