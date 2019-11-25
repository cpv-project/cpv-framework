#include <CPVFramework/Stream/PacketOutputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>
#include <CPVFramework/Utility/Macros.hpp>

namespace cpv {
	/** The storage of PacketOutputStream */
	template <>
	thread_local ReusableStorageType<PacketOutputStream>
		ReusableStorageInstance<PacketOutputStream>;
	
	/** Write data to stream */
	seastar::future<> PacketOutputStream::write(Packet&& data) {
		packet_->append(std::move(data));
		return seastar::make_ready_future<>();
	}
	
	/** For Reusable<> */
	void PacketOutputStream::freeResources() {
		packet_ = {};
	}
	
	/** For Reusable<> */
	void PacketOutputStream::reset(const seastar::lw_shared_ptr<Packet>& packet) {
		packet_ = packet;
	}
	
	/** Constructor */
	PacketOutputStream::PacketOutputStream() :
		packet_() { }
}

