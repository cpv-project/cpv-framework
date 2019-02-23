#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Utility/Macros.hpp>
#include "./Http11ServerConnectionRequestStream.hpp"
#include "./Http11ServerConnection.hpp"

namespace cpv {
	/** Read data from stream */
	seastar::future<InputStreamReadResult> Http11ServerConnectionRequestStream::read() {
		if (!returnedBody_ && connection_->temporaryData_.bodyBuffer.size() != 0) {
			// return the first part of initial body
			returnedBody_ = true;
			bool isEnd = (connection_->temporaryData_.messageCompleted &&
				returnedMoreBodyIndex_ >= connection_->temporaryData_.moreBodyBuffers.size());
			return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult(
				std::move(connection_->temporaryData_.bodyBuffer), isEnd));
		} else if (returnedMoreBodyIndex_ < connection_->temporaryData_.moreBodyBuffers.size()) {
			// return the rest parts of initial or following body
			std::size_t index = returnedMoreBodyIndex_++;
			bool isEnd = (connection_->temporaryData_.messageCompleted &&
				returnedMoreBodyIndex_ >= connection_->temporaryData_.moreBodyBuffers.size());
			return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult(
				std::move(connection_->temporaryData_.moreBodyBuffers[index]), isEnd));
		} else if (connection_->temporaryData_.messageCompleted) {
			// all body returned
			return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult());
		} else {
			// the message is incomplete, read more from connection
			// the following logic is almost same as Http11ServerConnection::receiveSingleRequest
			return connection_->socket_.in().read()
			.then([this] (seastar::temporary_buffer<char> tempBuffer) {
				// store the last buffer received
				auto& lastBuffer = connection_->temporaryData_.lastBuffer;
				lastBuffer = std::move(tempBuffer);
				// check whether connection is closed from remote
				if (lastBuffer.size() == 0) {
					connection_->state_ = Http11ServerConnectionState::Closing;
					return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult());
				}
				// clear previous body and reset read state
				auto& bodyBuffer = connection_->temporaryData_.bodyBuffer;
				auto& moreBodyBuffers = connection_->temporaryData_.moreBodyBuffers;
				returnedBody_ = true;
				returnedMoreBodyIndex_ = 0;
				bodyBuffer = {};
				moreBodyBuffers.clear();
				// execute http parser
				// lastBuffer may moved to bodyBuffer after parsed
				std::size_t lastBufferSize = lastBuffer.size();
				std::size_t parsedSize = ::http_parser_execute(
					&connection_->parser_,
					&connection_->parserSettings_,
					lastBuffer.get(),
					lastBufferSize);
				if (parsedSize != lastBufferSize) {
					auto err = static_cast<enum ::http_errno>(connection_->parser_.http_errno);
					if (err == ::http_errno::HPE_CB_message_begin &&
						connection_->temporaryData_.messageCompleted) {
						// received next request from pipeline
						if (CPV_LIKELY(bodyBuffer.size() != 0)) {
							connection_->nextRequestBuffer_ = bodyBuffer.share();
							connection_->nextRequestBuffer_.trim_front(parsedSize);
						} else if (lastBuffer.size() != 0) {
							connection_->nextRequestBuffer_ = lastBuffer.share();
							connection_->nextRequestBuffer_.trim_front(parsedSize);
						}
					} else {
						// parse error
						return connection_->replyErrorResponseForInvalidFormat().then([] {
							return InputStreamReadResult();
						});
					}
				}
				// return body
				bool isEnd = (connection_->temporaryData_.messageCompleted &&
					returnedMoreBodyIndex_ >= connection_->temporaryData_.moreBodyBuffers.size());
				return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult(
					std::move(connection_->temporaryData_.bodyBuffer), isEnd));
			});
		}
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
		connection_ = {};
	}
	
	/** For Object<> */
	void Http11ServerConnectionRequestStream::reset(
		seastar::shared_ptr<Http11ServerConnection> connection) {
		connection_ = std::move(connection);
		returnedBody_ = false;
		returnedMoreBodyIndex_ = 0;
	}
	
	/** Constructor */
	Http11ServerConnectionRequestStream::Http11ServerConnectionRequestStream() :
		connection_(),
		returnedBody_(),
		returnedMoreBodyIndex_() { }
}

