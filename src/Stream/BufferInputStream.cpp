#include <CPVFramework/Stream/BufferInputStream.hpp>

namespace cpv {
	/** Read data from stream */
	seastar::future<InputStreamReadResult> BufferInputStream::read() {
		if (buf_.size() > 0) {
			return seastar::make_ready_future<InputStreamReadResult>(
				InputStreamReadResult(std::move(buf_), true));
		} else {
			return seastar::make_ready_future<InputStreamReadResult>();
		}
	}
	
	/** For Object<> */
	void BufferInputStream::freeResources() {
		buf_ = {};
	}
	
	/** For Object<> */
	void BufferInputStream::reset(seastar::temporary_buffer<char>&& buf) {
		buf_ = std::move(buf);
	}
	
	/** Constructor */
	BufferInputStream::BufferInputStream() :
		buf_() { }
}

