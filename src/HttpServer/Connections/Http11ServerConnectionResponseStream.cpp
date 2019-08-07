#include <CPVFramework/Utility/Macros.hpp>
#include "./Http11ServerConnectionResponseStream.hpp"
#include "./Http11ServerConnection.hpp"

namespace cpv {
	/** The storage of Http11ServerConnectionRequestStream */
	template <>
	thread_local ReusableStorage<Http11ServerConnectionResponseStream>
		Reusable<Http11ServerConnectionResponseStream>::Storage(28232);
	
	/** Write data to stream */
	seastar::future<> Http11ServerConnectionResponseStream::write(seastar::net::packet&& data) {
		// reset detect timeout flag
		connection_->resetDetectTimeoutFlag();
		if (CPV_UNLIKELY(!static_cast<bool>(data))) {
			// ignore empty data
			return seastar::make_ready_future<>();
		} else if (CPV_UNLIKELY(connection_->replyLoopData_.responseHeadersAppended)) {
			// headers are sent, just send data
			connection_->replyLoopData_.responseWrittenBytes += data.len();
			return connection_->socket_.out().put(std::move(data));
		} else {
			// send headers and data in single packet (it's very important to performance)
			std::size_t fragmentsCount = connection_->getResponseHeadersFragmentsCount();
			fragmentsCount += data.nr_frags();
			seastar::net::packet merged(fragmentsCount);
			connection_->appendResponseHeaders(merged);
			connection_->replyLoopData_.responseWrittenBytes += data.len();
			merged.append(std::move(data));
			return connection_->socket_.out().put(std::move(merged));
		}
	}
	
	/** For Reusable<> */
	void Http11ServerConnectionResponseStream::freeResources() {
		connection_ = nullptr;
	}
	
	/** For Reusable<> */
	void Http11ServerConnectionResponseStream::reset(Http11ServerConnection* connection) {
		connection_ = connection;
	}
	
	/** Constructor */
	Http11ServerConnectionResponseStream::Http11ServerConnectionResponseStream() :
		connection_() { }
}

