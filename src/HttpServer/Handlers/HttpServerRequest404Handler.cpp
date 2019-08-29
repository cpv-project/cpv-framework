#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>

namespace cpv {
	/** Return 404 not found */
	seastar::future<> HttpServerRequest404Handler::handle(
		HttpContext& context,
		const HttpServerRequestHandlerIterator&) const {
		auto& response = context.response;
		response.setStatusCode(constants::_404);
		response.setStatusMessage(constants::NotFound);
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::ContentLength, constants::NotFound.size());
		return extensions::writeAll(response.getBodyStream(), constants::NotFound);
	}
}

