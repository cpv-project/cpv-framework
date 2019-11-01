#include <CPVFramework/Stream/BuffersInputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>

namespace cpv {
	/** The storage of BuffersInputStream */
	template <>
	thread_local ReusableStorageType<BuffersInputStream>
		ReusableStorageInstance<BuffersInputStream>;
	
	/** Read data from stream */
	seastar::future<InputStreamReadResult> BuffersInputStream::read() {
		while (index_ < buffers_.size()) {
			seastar::temporary_buffer<char> buffer = std::move(buffers_[index_++]);
			if (buffer.size() > 0) {
				// skip empty buffer
				return seastar::make_ready_future<InputStreamReadResult>(
					InputStreamReadResult(std::move(buffer), index_ >= buffers_.size()));
			}
		}
		return seastar::make_ready_future<InputStreamReadResult>();
	}
	
	/** Get the hint of total size of stream */
	std::optional<std::size_t> BuffersInputStream::sizeHint() const {
		return sizeHint_;
	}
	
	/** For Reusable<> */
	void BuffersInputStream::freeResources() {
		buffers_.clear();
	}
	
	/** For Reusable<> */
	void BuffersInputStream::reset(std::vector<seastar::temporary_buffer<char>>&& buffers) {
		buffers_ = std::move(buffers);
		sizeHint_ = 0;
		index_ = 0;
		for (auto& buffer : buffers_) {
			// overflow is ok because size hint can be different to actual size
			sizeHint_ += buffer.size();
		}
	}
	
	/** Constructor */
	BuffersInputStream::BuffersInputStream() :
		buffers_(),
		sizeHint_(0),
		index_(0) { }
}

