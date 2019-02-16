#include <CPVFramework/Exceptions/FormatException.hpp>
#include "./Http11ServerConnectionRequestStream.hpp"
#include "./Http11ServerConnection.hpp"

namespace cpv {
	/** Read data from stream */
	seastar::future<InputStreamReadResult> Http11ServerConnectionRequestStream::read() {
		/*if (!returnedBody_ && !connection_->parserTemporaryData_.bodyView.empty()) {
			// return the first part of initial body
			returnedBody_ = true;
			bool isEnd = (connection_->parserTemporaryData_.messageCompleted &&
				returnedMoreBodyIndex_ >= connection_->parserTemporaryData_.moreBodyViews.size());
			return seastar::make_ready_future<InputStreamReadResult>(
				InputStreamReadResult(connection_->parserTemporaryData_.bodyView, isEnd));
		} else if (returnedMoreBodyIndex_ < connection_->parserTemporaryData_.moreBodyViews.size()) {
			// return the rest parts of initial or following body
			std::size_t index = returnedMoreBodyIndex_;
			++returnedMoreBodyIndex_;
			seastar::temporary_buffer<char> buf;
			if (lastBuffer_.size() > 0) {
				buf = lastBuffer_.share();
			}
			bool isEnd = (connection_->parserTemporaryData_.messageCompleted &&
				returnedMoreBodyIndex_ >= connection_->parserTemporaryData_.moreBodyViews.size());
			return seastar::make_ready_future<InputStreamReadResult>(
				InputStreamReadResult(std::move(buf),
					connection_->parserTemporaryData_.moreBodyViews[index], isEnd));
		} else if (connection_->parserTemporaryData_.messageCompleted) {
			// all body returned
			return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult());
		} else {
			// message is not completed, read more from connection
			// the following logic is almost same as Http11ServerConnection::receiveSingleRequest
			return connection_->socket_.in().read()
			.then([this] (seastar::temporary_buffer<char> buf) {
				// check whether connection is closed from remote
				if (buf.size() == 0) {
					connection_->state_ = Http11ServerConnectionState::Closing;
					return seastar::make_ready_future<InputStreamReadResult>(InputStreamReadResult());
				}
				// clear previous body and reset read state
				lastBuffer_ = {};
				returnedBody_ = true;
				returnedMoreBodyIndex_ = 0;
				connection_->parserTemporaryData_.bodyView = {};
				connection_->parserTemporaryData_.moreBodyViews.clear();
				// execute http parser
				std::size_t parsedSize = ::http_parser_execute(
					&connection_->parser_,
					&connection_->parserSettings_,
					buf.get(),
					buf.size());
				if (parsedSize != buf.size()) {
					auto err = static_cast<enum ::http_errno>(connection_->parser_.http_errno);
					if (err == ::http_errno::HPE_CB_message_begin &&
						connection_->parserTemporaryData_.messageCompleted) {
						// received next request from pipeline
						connection_->nextRequestBuffer_ = buf.share();
						connection_->nextRequestBuffer_.trim_front(parsedSize);
					} else {
						// parse error
						return connection_->replyErrorResponseForInvalidFormat().then([] {
							return InputStreamReadResult();
						});
					}
				}
				// remember buffer if body contains multiple parts
				if (!connection_->parserTemporaryData_.moreBodyViews.empty()) {
					lastBuffer_ = buf.share();
				}
				// return body
				bool isEnd = (connection_->parserTemporaryData_.messageCompleted &&
					returnedMoreBodyIndex_ >= connection_->parserTemporaryData_.moreBodyViews.size());
				return seastar::make_ready_future<InputStreamReadResult>(
					InputStreamReadResult(connection_->parserTemporaryData_.bodyView, isEnd));
			});
		}*/
		// TODO rewrite here after implement custom allocators
		throw 1;
	}
	
	/** For Object<> */
	void Http11ServerConnectionRequestStream::freeResources() {
		connection_ = {};
		lastBuffer_ = {};
	}
	
	/** For Object<> */
	void Http11ServerConnectionRequestStream::reset(
		seastar::shared_ptr<Http11ServerConnection> connection) {
		connection_ = std::move(connection);
		lastBuffer_ = {};
		returnedBody_ = false;
		returnedMoreBodyIndex_ = 0;
	}
	
	/** Constructor */
	Http11ServerConnectionRequestStream::Http11ServerConnectionRequestStream() :
		connection_(),
		lastBuffer_(),
		returnedBody_(),
		returnedMoreBodyIndex_() { }
}

