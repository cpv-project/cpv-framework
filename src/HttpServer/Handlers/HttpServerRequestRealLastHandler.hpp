#pragma once
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestHandlerBase.hpp>

namespace cpv {
	/** The real last handler append from internal, it should never be invoked */
	class HttpServerRequestRealLastHandler : public HttpServerRequestHandlerBase {
	public:
		/** Return exception future */
		seastar::future<> handle(
			cpv::HttpRequest&,
			cpv::HttpResponse&,
			cpv::HttpServerRequestHandlerIterator) const override;
	};
}

