#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	/** Write data to stream */
	seastar::future<> StringOutputStream::write(const char* buf, std::size_t size) {
		if (CPV_LIKELY(str_.get() != nullptr)) {
			str_->append(buf, size);
		}
		return seastar::make_ready_future<>();
	}
	
	/** For Object<> */
	void StringOutputStream::freeResources() {
		str_ = {};
	}
	
	/** For Object<> */
	void StringOutputStream::reset(const seastar::lw_shared_ptr<std::string>& str) {
		str_ = str;
	}
	
	/** Constructor */
	StringOutputStream::StringOutputStream() :
		str_() { }
}

