#pragma once
#include <vector>
#include <seastar/core/temporary_buffer.hh>
#include "./InputStreamBase.hpp"

namespace cpv {
	/** Input stream that use given temporary buffers as data source */
	class BuffersInputStream : public InputStreamBase {
	public:
		/** Read data from stream */
		seastar::future<InputStreamReadResult> read() override;
		
		/** Get the hint of total size of stream */
		std::optional<std::size_t> sizeHint() const override;
		
		/** For Reusable<> */
		void freeResources();
		
		/** For Reusable<> */
		void reset(std::vector<seastar::temporary_buffer<char>>&& buffers);
		
		/** Constructor */
		BuffersInputStream();
		
	private:
		std::vector<seastar::temporary_buffer<char>> buffers_;
		std::size_t sizeHint_;
		std::size_t index_;
	};
}

