#pragma once
#include <seastar/core/future.hh>
#include <CPVFramework/Stream/InputStreamBase.hpp>

namespace cpv {
	/** Declare types */
	class Http11ServerConnection;
	
	/** The input stream for http 1.0/1.1 request */
	class Http11ServerConnectionRequestStream : public InputStreamBase {
	public:
		/** Read data from stream */
		seastar::future<InputStreamReadResult> read() override;
		
		/** Get the total size of stream */
		std::optional<std::size_t> size() const override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset(Http11ServerConnection* connection);
		
		/** Constructor */
		Http11ServerConnectionRequestStream();
		
	private:
		// the lifetime of stream is rely on the connection
		Http11ServerConnection* connection_;
		// whether is connection_->parserTemporaryData_.bodyBuffer returned
		bool returnedBody_;
		// the last index of connection_->parserTemporaryData_.moreBodyBuffers returned
		std::size_t returnedMoreBodyIndex_;
	};
}

