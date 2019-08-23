#pragma once
#include <seastar/core/future.hh>
#include "../Utility/Packet.hpp"

namespace cpv {
	/**
	 * Interface of simple output stream.
	 * The write function will take a cpv::Packet (may contains multiple segments) and
	 * write the data of segments to stream.
	 */
	class OutputStreamBase {
	public:
		/** Virtual destructor */
		virtual ~OutputStreamBase() = default;
		
		/** Write data to stream */
		virtual seastar::future<> write(Packet&& data) = 0;
	};
}

