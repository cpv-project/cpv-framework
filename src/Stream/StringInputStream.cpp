#include <CPVFramework/Stream/StringInputStream.hpp>

namespace cpv {
	/** Read data from stream */
	seastar::future<InputStreamReadResult> StringInputStream::read(char* buf, std::size_t size) {
		if (position_ >= str_.size()) {
			return seastar::make_ready_future<InputStreamReadResult>();
		}
		std::size_t copy_max = str_.size() - position_;
		std::size_t copy_n = std::min(copy_max, size);
		std::memcpy(buf, str_.data() + position_, copy_n);
		return seastar::make_ready_future<InputStreamReadResult>(
			InputStreamReadResult(copy_n, copy_n == copy_max));
	}
	
	/** For Object<> */
	void StringInputStream::freeResources() {
		str_.resize(0);
	}
	
	/** For Object<> */
	void StringInputStream::reset(std::string&& str) {
		str_ = std::move(str);
		position_ = 0;
	}
	
	/** Constructor */
	StringInputStream::StringInputStream() :
		str_(),
		position_() { }
}

