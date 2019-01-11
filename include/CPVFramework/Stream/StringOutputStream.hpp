#pragma once
#include <seastar/core/shared_ptr.hh>
#include "./OutputStream.hpp"

namespace cpv {
	/** Output stream that use given string as data destination */
	class StringOutputStream : public OutputStream {
	public:
		/** Write data to stream, the buffer must live until future resolved */
		seastar::future<> write(char* buf, std::size_t size) override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset(const seastar::lw_shared_ptr<std::string>& str);
		
		/** Constructor */
		StringOutputStream();
		
	private:
		seastar::lw_shared_ptr<std::string> str_;
	};
}

