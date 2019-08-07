#pragma once
#include <seastar/core/future.hh>
#include <CPVFramework/Stream/InputStreamBase.hpp>
#include <CPVFramework/Utility/Reusable.hpp>

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
		
		/** For Reusable<> */
		void freeResources();
		
		/** For Reusable<> */
		void reset(Http11ServerConnection* connection);
		
		/** Constructor */
		Http11ServerConnectionRequestStream();
		
	private:
		// the lifetime of stream is rely on the connection
		Http11ServerConnection* connection_;
	};
}

