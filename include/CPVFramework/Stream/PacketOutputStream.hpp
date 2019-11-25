#pragma once
#include "../Utility/Packet.hpp"
#include "./OutputStreamBase.hpp"

namespace cpv {
	/** Output stream that use given packet as data sink */
	class PacketOutputStream : public OutputStreamBase {
	public:
		/** Write data to stream */
		seastar::future<> write(Packet&& data) override;
		
		/** For Reusable<> */
		void freeResources();
		
		/** For Reusable<> */
		void reset(const seastar::lw_shared_ptr<Packet>& packet);
		
		/** Constructor */
		PacketOutputStream();
		
	private:
		seastar::lw_shared_ptr<Packet> packet_;
	};
}

