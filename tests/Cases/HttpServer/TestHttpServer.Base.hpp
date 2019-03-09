#pragma once
#include <seastar/core/future.hh>
#include <CPVFramework/HttpServer/HttpServer.hpp>

namespace cpv::gtest {
	struct HttpServerTestFunctions {
		std::function<void(cpv::HttpServerConfiguration&)> updateConfiguration;
		std::function<cpv::HttpServerRequestHandlerCollection()> makeHandlers;
		std::function<seastar::future<>()> execute;
	};
	
	/** Generic test runner for http server */
	seastar::future<> runHttpServerTest(HttpServerTestFunctions&& testFunctions);
	
	/** Handler that reply url and header values in response body */
	class HttpCheckHeadersHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpRequest& request,
			cpv::HttpResponse& response,
			const cpv::HttpServerRequestHandlerIterator&) const override;
	};
}

