#include <CPVFramework/Stream/BufferInputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>

namespace cpv {
	/** The storage of BufferInputStream */
	template <>
	thread_local ReusableStorageType<BufferInputStream>
		ReusableStorageInstance<BufferInputStream>;
	
	/** Read data from stream */
	seastar::future<InputStreamReadResult> BufferInputStream::read() {
		if (buf_.size() > 0) {
			return seastar::make_ready_future<InputStreamReadResult>(
				InputStreamReadResult(std::move(buf_), true));
		} else {
			return seastar::make_ready_future<InputStreamReadResult>();
		}
	}
	
	/** Get the hint of total size of stream */
	std::optional<std::size_t> BufferInputStream::sizeHint() const {
		return sizeHint_;
	}
	
	/** For Reusable<> */
	void BufferInputStream::freeResources() {
		buf_ = {};
	}
	
	/** For Reusable<> */
	void BufferInputStream::reset(seastar::temporary_buffer<char>&& buf) {
		buf_ = std::move(buf);
		sizeHint_ = buf_.size();
	}
	
	/** Constructor */
	BufferInputStream::BufferInputStream() :
		buf_(),
		sizeHint_(0) { }
}

