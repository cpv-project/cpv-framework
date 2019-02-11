#include <CPVFramework/Stream/StringInputStream.hpp>

namespace cpv {
	/** Read data from stream */
	seastar::future<InputStreamReadResult> StringInputStream::read() {
		if (isEnd_) {
			return seastar::make_ready_future<InputStreamReadResult>();
		} else {
			isEnd_ = true;
			return seastar::make_ready_future<InputStreamReadResult>(
				InputStreamReadResult(str_, true));
		}
	}
	
	/** For Object<> */
	void StringInputStream::freeResources() {
		str_.resize(0);
	}
	
	/** For Object<> */
	void StringInputStream::reset(std::string&& str) {
		str_ = std::move(str);
		isEnd_ = false;
	}
	
	/** Constructor */
	StringInputStream::StringInputStream() :
		str_(),
		isEnd_() { }
}

