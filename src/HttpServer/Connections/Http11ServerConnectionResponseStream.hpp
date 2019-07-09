#pragma once
#include <seastar/core/future.hh>
#include <CPVFramework/Stream/OutputStreamBase.hpp>

namespace cpv {
	/** Declare types */
	class Http11ServerConnection;
	
	/** The output stream for http 1.0/1.1 response */
	class Http11ServerConnectionResponseStream : public OutputStreamBase {
	public:
		/**
		 * Write data to stream.
		 * This function didn't handle chunked data encoding,
		 * (for performance, add size and crlf to packet may trigger unnecessary allocation)
		 * chunked data should be encoded from writer.
		 */
		seastar::future<> write(seastar::net::packet&& data) override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset(Http11ServerConnection* connection);
		
		/** Constructor */
		Http11ServerConnectionResponseStream();
		
	private:
		// the lifetime of stream is rely on the connection
		Http11ServerConnection* connection_;
	};
}

