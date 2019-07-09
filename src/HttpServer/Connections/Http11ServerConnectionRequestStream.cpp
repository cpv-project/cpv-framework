#include <CPVFramework/Exceptions/LogicException.hpp>
#include "./Http11ServerConnectionRequestStream.hpp"
#include "./Http11ServerConnection.hpp"

namespace cpv {
	/** Read data from stream */
	seastar::future<InputStreamReadResult> Http11ServerConnectionRequestStream::read() {
		if (connection_->replyLoopData_.requestBodyConsumed) {
			return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult());
		}
		return connection_->requestBodyQueue_.pop_eventually().then([this] (auto bodyEntry) {
			if (CPV_UNLIKELY(bodyEntry.id != connection_->replyLoopData_.requestId)) {
				return seastar::make_exception_future<InputStreamReadResult>(
					LogicException(CPV_CODEINFO, "request id not matched, body belongs to request",
					bodyEntry.id, "but processing request is", connection_->replyLoopData_.requestId));
			}
			if (bodyEntry.isEnd) {
				connection_->replyLoopData_.requestBodyConsumed = true;
			}
			return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult(
				std::move(bodyEntry.buffer), bodyEntry.isEnd));
		});
	}
	
	/** Get the total size of stream */
	std::optional<std::size_t> Http11ServerConnectionRequestStream::size() const {
		std::size_t contentLength = connection_->parser_.content_length;
		if (CPV_LIKELY(contentLength > 0)) {
			return contentLength;
		} else {
			return { };
		}
	}
	
	/** For Object<> */
	void Http11ServerConnectionRequestStream::freeResources() {
		connection_ = nullptr;
	}
	
	/** For Object<> */
	void Http11ServerConnectionRequestStream::reset(Http11ServerConnection* connection) {
		connection_ = connection;
	}
	
	/** Constructor */
	Http11ServerConnectionRequestStream::Http11ServerConnectionRequestStream() :
		connection_() { }
}

