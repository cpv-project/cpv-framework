#include <CPVFramework/HttpServer/Handlers/HttpServerRequest404Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>

namespace cpv {
	/** Return 404 not found */
	seastar::future<> HttpServerRequest404Handler::handle(
		HttpRequest&,
		HttpResponse& response,
		const HttpServerRequestHandlerIterator&) const {
		response.setStatusCode(constants::_404);
		response.setStatusMessage(constants::NotFound);
		// TODO: set from http connection
		// response.setHeader(constants::Date, formatNowForHttpHeader());
		response.setHeader(constants::ContentType, constants::TextPlainUtf8);
		response.setHeader(constants::ContentLength,
			constants::Integers.at(constants::NotFound.size()));
		return extensions::writeAll(response.getBodyStream(), constants::NotFound);
	}
}

