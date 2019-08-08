#pragma once
#include <seastar/core/shared_ptr.hh>
#include "./OutputStreamBase.hpp"

namespace cpv {
	/** Output stream that use given string as data destination */
	class StringOutputStream : public OutputStreamBase {
	public:
		/** Write data to stream */
		seastar::future<> write(seastar::net::packet&& data) override;
		
		/** For Reusable<> */
		void freeResources();
		
		/** For Reusable<> */
		void reset(const seastar::lw_shared_ptr<std::string>& str);
		
		/** Constructor */
		StringOutputStream();
		
	private:
		seastar::lw_shared_ptr<std::string> str_;
	};
}

