#pragma once
#include "../Utility/SharedStringBuilder.hpp"
#include "./OutputStreamBase.hpp"

namespace cpv {
	/** Output stream that use given string builder as data sink */
	class StringOutputStream : public OutputStreamBase {
	public:
		/** Write data to stream */
		seastar::future<> write(Packet&& data) override;
		
		/** For Reusable<> */
		void freeResources();
		
		/** For Reusable<> */
		void reset(const seastar::lw_shared_ptr<SharedStringBuilder>& builder);
		
		/** Constructor */
		StringOutputStream();
		
	private:
		seastar::lw_shared_ptr<SharedStringBuilder> builder_;
	};
}

