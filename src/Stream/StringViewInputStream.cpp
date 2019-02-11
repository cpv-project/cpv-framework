#include <CPVFramework/Stream/StringViewInputStream.hpp>

namespace cpv {
	/** Read data from stream */
	seastar::future<InputStreamReadResult> StringViewInputStream::read() {
		if (isEnd_) {
			return seastar::make_ready_future<InputStreamReadResult>();
		} else {
			isEnd_ = true;
			return seastar::make_ready_future<InputStreamReadResult>(
				InputStreamReadResult(str_, true));
		}
	}
	
	/** For Object<> */
	void StringViewInputStream::freeResources() {
		str_ = {};
	}
	
	/** For Object<> */
	void StringViewInputStream::reset(const std::string_view& str) {
		str_ = str;
		isEnd_ = false;
	}
	
	/** Constructor */
	StringViewInputStream::StringViewInputStream() :
		str_(),
		isEnd_() { }
}

