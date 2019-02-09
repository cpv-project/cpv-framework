#pragma once
#include <seastar/core/future.hh>
#include <CPVFramework/Stream/InputStreamBase.hpp>

namespace cpv {
	/** The input stream for http 1.0/1.1 request */
	class Http11ServerConnectionRequestStream : public InputStreamBase {
	public:
		seastar::future<InputStreamReadResult> read(char* buf, std::size_t size) override;
		
		static void freeResources();
		
		static void reset();
		
		Http11ServerConnectionRequestStream();
		
	private:
	};
}

