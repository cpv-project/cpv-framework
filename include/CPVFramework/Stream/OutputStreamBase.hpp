#pragma once
#include <seastar/core/future.hh>
#include <seastar/net/packet.hh>

namespace cpv {
	/** Interface of simple output stream */
	class OutputStreamBase {
	public:
		/** Virtual destructor */
		virtual ~OutputStreamBase() = default;
		
		/** Write data to stream */
		virtual seastar::future<> write(seastar::net::packet&& data) = 0;
	};
}

