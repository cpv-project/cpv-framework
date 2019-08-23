#include <CPVFramework/Utility/Macros.hpp>
#include "./Http11ServerConnectionResponseStream.hpp"
#include "./Http11ServerConnection.hpp"

namespace cpv {
	/** The storage of Http11ServerConnectionRequestStream */
	template <>
	thread_local ReusableStorageType<Http11ServerConnectionResponseStream>
		ReusableStorageInstance<Http11ServerConnectionResponseStream>;
	
	/** Write data to stream */
	seastar::future<> Http11ServerConnectionResponseStream::write(Packet&& data) {
		// reset detect timeout flag
		connection_->resetDetectTimeoutFlag();
		if (CPV_UNLIKELY(connection_->replyLoopData_.responseHeadersAppended)) {
			// headers are sent, just send data
			connection_->replyLoopData_.responseWrittenBytes += data.size();
			return connection_->socket_.out() << std::move(data);
		} else {
			// send headers and data in single packet (it's very important to performance)
			Packet merged(connection_->getResponseHeadersFragmentsCount() + data.segments());
			connection_->appendResponseHeaders(merged);
			connection_->replyLoopData_.responseWrittenBytes += data.size();
			merged.append(std::move(data));
			return connection_->socket_.out() << std::move(merged);
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

