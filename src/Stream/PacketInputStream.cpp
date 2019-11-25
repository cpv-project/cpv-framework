#include <CPVFramework/Stream/PacketInputStream.hpp>
#include <CPVFramework/Utility/Reusable.hpp>

namespace cpv {
	/** The storage of PacketInputStream */
	template <>
	thread_local ReusableStorageType<PacketInputStream>
		ReusableStorageInstance<PacketInputStream>;
	
	/** Read data from stream */
	seastar::future<InputStreamReadResult> PacketInputStream::read() {
		if (auto ptr = packet_.getIfMultiple()) {
			while (index_ < ptr->fragments.size()) {
				auto& fragment = ptr->fragments[index_];
				++index_;
				if (fragment.size == 0) {
					continue;
				}
				return seastar::make_ready_future<InputStreamReadResult>(
					InputStreamReadResult(
						SharedString(fragment.base,
							fragment.size, ptr->deleter.share()),
						index_ >= ptr->fragments.size()));
			}
		} else if (auto ptr = packet_.getIfSingle()) {
			if (index_ == 0) {
				index_ = 1;
				return seastar::make_ready_future<InputStreamReadResult>(
					InputStreamReadResult(
						SharedString(ptr->fragment.base,
							ptr->fragment.size, ptr->deleter.share()),
						true));
			}
		}
		return seastar::make_ready_future<InputStreamReadResult>();
	}
	
	/** Get the hint of total size of stream */
	std::optional<std::size_t> PacketInputStream::sizeHint() const {
		return sizeHint_;
	}
	
	/** For Reusable<> */
	void PacketInputStream::freeResources() {
		packet_ = {};
	}
	
	/** For Reusable<> */
	void PacketInputStream::reset(Packet&& packet) {
		packet_ = std::move(packet);
		sizeHint_ = packet_.size();
		index_ = 0;
	}
	
	/** Constructor */
	PacketInputStream::PacketInputStream() :
		packet_(),
		sizeHint_(0),
		index_(0) { }
}

