#include <CPVFramework/HttpServer/Handlers/HttpServerRequest500Handler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Stream/OutputStreamExtensions.hpp>
#include <CPVFramework/Utility/UUIDUtils.hpp>

namespace cpv {
	/** Return 500 internal server error */
	seastar::future<> HttpServerRequest500Handler::handle(
		HttpRequest& request,
		HttpResponse& response,
		HttpServerRequestHandlerIterator next) const {
		return (*next)->handle(request, response, next + 1)
			.handle_exception([&response, this] (std::exception_ptr ex) {
			// generate a time uuid as error id
			std::string uuidStr(uuidToStr(makeTimeUUID()));
			// log error
			logger_->log(LogLevel::Error, "Http server request error, ID:", uuidStr, "\n", ex);
			// build response content
			std::string content;
			content.append(constants::InternalServerError)
				.append("\n")
				.append("ID: ")
				.append(uuidStr);
			// set 500 status code (headers may already write to client,
			// in this case the error message will append to content and the behavior is undefined,
			// most of the time user can see it in network tab of developer tool)
			response.setStatusCode(constants::_500);
			response.setStatusMessage(constants::InternalServerError);
			response.setHeader(constants::ContentType, constants::TextPlainUtf8);
			response.setHeader(constants::ContentLength, constants::Integers.at(content.size()));
			// write response content
			seastar::temporary_buffer buf(content.data(), content.size());
			std::string_view bufView(buf.get(), buf.size());
			response.addUnderlyingBuffer(std::move(buf));
			return extensions::writeAll(response.getBodyStream(), bufView);
		});
	}
	
	/** Constructor */
	HttpServerRequest500Handler::HttpServerRequest500Handler(
		const seastar::shared_ptr<cpv::Logger> logger) :
		logger_(logger) { }
}

