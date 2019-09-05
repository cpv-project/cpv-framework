#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Utility/UUIDUtils.hpp>

namespace cpv {
	namespace {
		seastar::future<> reply500Response(
			HttpContext& context,
			std::exception_ptr ex) {
			auto& response = context.getResponse();
			auto logger = context.getService<seastar::shared_ptr<Logger>>();
			// generate a time uuid as error id
			std::string uuidStr(uuidToStr(makeTimeUUID()));
			// log error
			logger->log(LogLevel::Error, "Http server request error, ID:", uuidStr, "\n", ex);
			// build response content
			Packet p;
			p.append(constants::InternalServerError)
				.append("\nID: ")
				.append(seastar::temporary_buffer<char>(uuidStr.data(), uuidStr.size()));
			// set 500 status code (headers may already write to client,
			// in this case the error message will append to content and the behavior is undefined,
			// most of the time user can see it in network tab of developer tool)
			response.setStatusCode(constants::_500);
			response.setStatusMessage(constants::InternalServerError);
			response.setHeader(constants::ContentType, constants::TextPlainUtf8);
			response.setHeader(constants::ContentLength, p.size());
			// write response content
			return extensions::writeAll(response.getBodyStream(), std::move(p));
		}
	}
	
	/** Return 500 internal server error */
	seastar::future<> HttpServerRequest500Handler::handle(
		HttpContext& context,
		const HttpServerRequestHandlerIterator& next) const {
		// expand futurize_apply manually to reduce overhead
		try {
			auto f = (*next)->handle(context, next + 1);
			if (f.available() && !f.failed()) {
				return f;
			}
			return f.handle_exception([&context, this] (std::exception_ptr ex) {
				return reply500Response(context, ex);
			});
		} catch (...) {
			return reply500Response(context, std::current_exception());
		}
	}
}

