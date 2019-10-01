#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>

namespace cpv {
	/** Return 404 not found */
	seastar::future<> HttpServerRequest404Handler::handle(
		HttpContext& context,
		const HttpServerRequestHandlerIterator&) const {
		auto& response = context.getResponse();
		return extensions::reply(response, constants::NotFound,
			constants::TextPlainUtf8, constants::_404, constants::NotFound);
	}
}

