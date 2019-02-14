#pragma once
#include <seastar/core/future.hh>
#include <seastar/core/shared_ptr.hh>
#include <CPVFramework/Stream/OutputStreamBase.hpp>

namespace cpv {
	/** Declare types */
	class Http11ServerConnection;
	
	/** The output stream for http 1.0/1.1 response */
	class Http11ServerConnectionResponseStream : public OutputStreamBase {
	public:
		/** Write data to stream */
		seastar::future<> write(const char* buf, std::size_t size) override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset();
		
		/** Constructor */
		Http11ServerConnectionResponseStream();
		
	private:
		seastar::shared_ptr<Http11ServerConnection> connection_;
	};
}

