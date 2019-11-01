#include <CPVFramework/Exceptions/LogicException.hpp>
#include "./Http11ServerConnectionRequestStream.hpp"
#include "./Http11ServerConnection.hpp"

namespace cpv {
	/** The storage of Http11ServerConnectionRequestStream */
	template <>
	thread_local ReusableStorageType<Http11ServerConnectionRequestStream>
		ReusableStorageInstance<Http11ServerConnectionRequestStream>;
	
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
	
	/** Get the hint of total size of stream */
	std::optional<std::size_t> Http11ServerConnectionRequestStream::sizeHint() const {
		// malicious client may send a smaller content-length to cause buffer overflow,
		// but this is just a hint.
		std::size_t contentLength = connection_->parser_.content_length;
		if (CPV_LIKELY(contentLength > 0 &&
			contentLength != std::numeric_limits<uint64_t>::max())) {
			return contentLength;
		} else {
			return { };
		}
	}
	
	/** For Reusable<> */
	void Http11ServerConnectionRequestStream::freeResources() {
		connection_ = nullptr;
	}
	
	/** For Reusable<> */
	void Http11ServerConnectionRequestStream::reset(Http11ServerConnection* connection) {
		connection_ = connection;
	}
	
	/** Constructor */
	Http11ServerConnectionRequestStream::Http11ServerConnectionRequestStream() :
		connection_() { }
}

