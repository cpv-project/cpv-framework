#include <CPVFramework/Utility/Macros.hpp>
#include "./Http11ServerConnectionResponseStream.hpp"
#include "./Http11ServerConnection.hpp"

namespace cpv {
	/** Write data to stream */
	seastar::future<> Http11ServerConnectionResponseStream::write(seastar::net::packet&& data) {
		if (CPV_UNLIKELY(!static_cast<bool>(data))) {
			// ignore empty data
			return seastar::make_ready_future<>();
		} else if (connection_->temporaryData_.responseHeadersFlushed) {
			// headers are flushed, send data directly
			connection_->temporaryData_.responseBodyWrittenSize += data.len();
			return connection_->socket_.out().put(std::move(data));
		} else {
			// flush headers first, then send data directly
			return connection_->flushResponseHeaders().then([this, d=std::move(data)]() mutable {
				connection_->temporaryData_.responseBodyWrittenSize += d.len();
				return connection_->socket_.out().put(std::move(d));
			});
		}
	}
	
	/** For Object<> */
	void Http11ServerConnectionResponseStream::freeResources() {
		connection_ = nullptr;
	}
	
	/** For Object<> */
	void Http11ServerConnectionResponseStream::reset(Http11ServerConnection* connection) {
		connection_ = connection;
	}
	
	/** Constructor */
	Http11ServerConnectionResponseStream::Http11ServerConnectionResponseStream() :
		connection_() { }
}

