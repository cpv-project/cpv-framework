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
	
	/** Handler that echo request url and header values in response body */
	class HttpCheckHeadersHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpContext& context,
			const cpv::HttpServerRequestHandlerIterator&) const override;
	};
	
	/** Handler that echo request body in response body */
	class HttpCheckBodyHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpContext& context,
			const cpv::HttpServerRequestHandlerIterator&) const override;
	};
	
	/** Handler that reply response with body but without content length header */
	class HttpLengthNotFixedHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpContext& context,
			const cpv::HttpServerRequestHandlerIterator&) const override;
	};
	
	/** Handler that reply response with body but size not matched to content length header */
	class HttpWrittenSizeNotMatchedHandler : public cpv::HttpServerRequestHandlerBase {
	public:
		seastar::future<> handle(
			cpv::HttpContext& context,
			const cpv::HttpServerRequestHandlerIterator&) const override;
	};
}

