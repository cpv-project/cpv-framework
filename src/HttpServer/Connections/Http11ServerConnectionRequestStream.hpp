#pragma once
#include <seastar/core/future.hh>
#include <seastar/core/shared_ptr.hh>
#include <CPVFramework/Stream/InputStreamBase.hpp>

namespace cpv {
	/** Declare types */
	class Http11ServerConnection;
	
	/** The input stream for http 1.0/1.1 request */
	class Http11ServerConnectionRequestStream : public InputStreamBase {
	public:
		/** Read data from stream */
		seastar::future<InputStreamReadResult> read() override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset(seastar::shared_ptr<Http11ServerConnection> connection);
		
		/** Constructor */
		Http11ServerConnectionRequestStream();
		
	private:
		seastar::shared_ptr<Http11ServerConnection> connection_;
		// store buffer for body that contains multiple parts
		seastar::temporary_buffer<char> lastBuffer_;
		// whether is connection_->parserTemporaryData_.bodyView returned
		bool returnedBody_;
		// the last index of connection_->parserTemporaryData_.moreBodyViews returned
		std::size_t returnedMoreBodyIndex_;
	};
}

