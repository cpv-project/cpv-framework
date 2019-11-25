#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Utility/UUIDUtils.hpp>

namespace cpv {
	namespace {
		seastar::future<> reply500Response(
			HttpContext& context,
			std::exception_ptr ex,
			const seastar::shared_ptr<Logger>& logger) {
			auto& response = context.getResponse();
			// generate a time uuid as error id
			SharedString uuidStr = uuidToStr(makeTimeUUID());
			// log error
			logger->log(LogLevel::Error, "Http server request error, ID:", uuidStr, "\n", ex);
			// build response content
			Packet p;
			p.append(constants::InternalServerError).append("\nID: ").append(std::move(uuidStr));
			// reply with 500 (headers may already write to client,
			// in this case the error message will append to content and the behavior is undefined,
			// most of the time user can see it in network tab of developer tool)
			return extensions::reply(response, std::move(p),
				constants::TextPlainUtf8, constants::_500, constants::InternalServerError);
		}
	}
	
	/** Return 500 internal server error */
	seastar::future<> HttpServerRequest500Handler::handle(
		HttpContext& context,
		HttpServerRequestHandlerIterator next) const {
		// expand futurize_apply manually to reduce overhead
		try {
			auto f = (*next)->handle(context, next + 1);
			if (f.available() && !f.failed()) {
				return f;
			}
			return f.handle_exception([&context, this] (std::exception_ptr ex) {
				return reply500Response(context, ex, logger_);
			});
		} catch (...) {
			return reply500Response(context, std::current_exception(), logger_);
		}
	}

	/** Constructor */
	HttpServerRequest500Handler::HttpServerRequest500Handler(
		seastar::shared_ptr<Logger> logger) :
		logger_(std::move(logger)) { }
}

