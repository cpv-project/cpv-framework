#pragma once
#include <seastar/core/future.hh>

namespace cpv {
	/** Interface of simple output stream */
	class OutputStream {
	public:
		/** Write data to stream, the buffer must live until future resolved */
		virtual seastar::future<> write(char* buf, std::size_t size) = 0;
	};
}

