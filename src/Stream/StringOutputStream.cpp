#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	/** Write data to stream */
	seastar::future<> StringOutputStream::write(seastar::net::packet&& data) {
		if (CPV_LIKELY(str_.get() != nullptr)) {
			for (std::size_t i = 0, j = data.nr_frags(); i < j; ++i) {
				auto& fragment = data.frag(i);
				str_->append(fragment.base, fragment.size);
			}
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

