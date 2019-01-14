#include <CPVFramework/Stream/StringViewInputStream.hpp>

namespace cpv {
	/** Read data from stream */
	seastar::future<InputStreamReadResult> StringViewInputStream::read(char* buf, std::size_t size) {
		if (position_ >= str_.size()) {
			return seastar::make_ready_future<InputStreamReadResult>();
		}
		std::size_t copy_max = str_.size() - position_;
		std::size_t copy_n = std::min(copy_max, size);
		std::memcpy(buf, str_.data() + position_, copy_n);
		position_ += copy_n;
		return seastar::make_ready_future<InputStreamReadResult>(
			InputStreamReadResult(copy_n, copy_n == copy_max));
	}
	
	/** For Object<> */
	void StringViewInputStream::freeResources() {
		str_ = {};
	}
	
	/** For Object<> */
	void StringViewInputStream::reset(const std::string_view& str) {
		str_ = str;
		position_ = 0;
	}
	
	/** Constructor */
	StringViewInputStream::StringViewInputStream() :
		str_(),
		position_() { }
}

