#pragma once
#include "../Utility/Packet.hpp"
#include "./InputStreamBase.hpp"

namespace cpv {
	/** Input stream that use given packet as data source */
	class PacketInputStream : public InputStreamBase {
	public:
		/** Read data from stream */
		seastar::future<InputStreamReadResult> read() override;
		
		/** Get the hint of total size of stream */
		std::optional<std::size_t> sizeHint() const override;
		
		/** For Reusable<> */
		void freeResources();
		
		/** For Reusable<> */
		void reset(Packet&& packet);
		
		/** Constructor */
		PacketInputStream();
		
	private:
		Packet packet_;
		std::size_t sizeHint_;
		std::size_t index_;
	};
}

