#pragma once
#include <seastar/core/future.hh>
#include <CPVFramework/Stream/OutputStreamBase.hpp>

namespace cpv {
	/** The output stream for http 1.0/1.1 response */
	class Http11ServerConnectionResponseStream : public OutputStreamBase {
	public:
		seastar::future<> write(const char* buf, std::size_t size) override;
		
		static void freeResources();
		
		static void reset();
		
		Http11ServerConnectionResponseStream();
		
	private:
	};
}

