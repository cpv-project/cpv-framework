#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>

namespace cpv {
	/** The storage of StringInputStream */
	template <>
	thread_local ReusableStorageType<StringInputStream>
		ReusableStorageInstance<StringInputStream>;
	
	/** Read data from stream */
	seastar::future<InputStreamReadResult> StringInputStream::read() {
		if (isEnd_) {
			return seastar::make_ready_future<InputStreamReadResult>();
		} else {
			// notice the lifetime of data is bound to stream
			isEnd_ = true;
			seastar::temporary_buffer<char> buf(str_.data(), str_.size(), seastar::deleter());
			return seastar::make_ready_future<InputStreamReadResult>(
				InputStreamReadResult(std::move(buf), true));
		}
	}
	
	/** Get the total size of stream */
	std::optional<std::size_t> StringInputStream::size() const {
		return str_.size();
	}
	
	/** For Reusable<> */
	void StringInputStream::freeResources() {
		str_.resize(0);
	}
	
	/** For Reusable<> */
	void StringInputStream::reset(std::string&& str) {
		str_ = std::move(str);
		isEnd_ = false;
	}
	
	/** Constructor */
	StringInputStream::StringInputStream() :
		str_(),
		isEnd_() { }
}

