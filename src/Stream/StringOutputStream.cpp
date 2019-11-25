#include <CPVFramework/Stream/StringOutputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	/** The storage of StringOutputStream */
	template <>
	thread_local ReusableStorageType<StringOutputStream>
		ReusableStorageInstance<StringOutputStream>;
	
	/** Write data to stream */
	seastar::future<> StringOutputStream::write(Packet&& data) {
		if (auto ptr = data.getIfSingle()) {
			auto& fragment = ptr->fragment;
			builder_->append({ fragment.base, fragment.size });
		} else if (auto ptr = data.getIfMultiple()) {
			for (auto& fragment : ptr->fragments) {
				builder_->append({ fragment.base, fragment.size });
			}
		}
		return seastar::make_ready_future<>();
	}
	
	/** For Reusable<> */
	void StringOutputStream::freeResources() {
		builder_ = {};
	}
	
	/** For Reusable<> */
	void StringOutputStream::reset(
		const seastar::lw_shared_ptr<SharedStringBuilder>& builder) {
		builder_ = builder;
	}
	
	/** Constructor */
	StringOutputStream::StringOutputStream() :
		builder_() { }
}

