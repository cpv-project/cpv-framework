#pragma once
#include <seastar/core/future.hh>

namespace cpv {
	/** Interface of simple input stream */
	class InputStream {
	public:
		/** Contains actual read size and is end of stream */
		struct ReadResult {
			std::size_t size;
			bool eof;
			
			ReadResult(std::size_t sizeVal, bool eofVal) :
				size(sizeVal), eof(eofVal) { }
			ReadResult() : ReadResult(0, true) { }
		};
		
		/** Read data from stream, the buffer must live until future resolved */
		virtual seastar::future<ReadResult> read(char* buf, std::size_t size) = 0;
	};
}

