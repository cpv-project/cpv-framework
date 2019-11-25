#include <CPVFramework/Stream/StringInputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>

namespace cpv {
	/** The storage of StringInputStream */
	template <>
	thread_local ReusableStorageType<StringInputStream>
		ReusableStorageInstance<StringInputStream>;
	
	/** Read data from stream */
	seastar::future<InputStreamReadResult> StringInputStream::read() {
		if (str_.empty()) {
			return seastar::make_ready_future<InputStreamReadResult>();
		} else {
			return seastar::make_ready_future<InputStreamReadResult>(
				InputStreamReadResult(std::move(str_), true));
		}
	}
	
	/** Get the hint of total size of stream */
	std::optional<std::size_t> StringInputStream::sizeHint() const {
		return sizeHint_;
	}
	
	/** For Reusable<> */
	void StringInputStream::freeResources() {
		str_ = {};
	}
	
	/** For Reusable<> */
	void StringInputStream::reset(SharedString&& str) {
		str_ = std::move(str);
		sizeHint_ = str_.size();
	}
	
	/** Constructor */
	StringInputStream::StringInputStream() :
		str_(),
		sizeHint_(0) { }
}

