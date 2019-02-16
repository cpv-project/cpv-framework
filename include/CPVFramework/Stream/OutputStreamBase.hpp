#pragma once
#include <seastar/core/future.hh>
#include <seastar/net/packet.hh>

namespace cpv {
	/**
	 * Interface of simple output stream.
	 * The write function will take a packet (may contains multiple segments) and
	 * write the data of segments to stream.
	 * seastar::scattered_message can build a packet with multiple segments, useful for reducing copy.
	 */
	class OutputStreamBase {
	public:
		/** Virtual destructor */
		virtual ~OutputStreamBase() = default;
		
		/** Write data to stream */
		virtual seastar::future<> write(seastar::net::packet&& data) = 0;
	};
}

