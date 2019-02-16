#pragma once
#include <seastar/core/temporary_buffer.hh>
#include "./InputStreamBase.hpp"

namespace cpv {
	/** Input stream that use given temporary buffer as data source */
	class BufferInputStream : public InputStreamBase {
	public:
		/** Read data from stream */
		seastar::future<InputStreamReadResult> read() override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset(seastar::temporary_buffer<char>&& buf);
		
		/** Constructor */
		BufferInputStream();
		
	private:
		seastar::temporary_buffer<char> buf_;
	};
}

