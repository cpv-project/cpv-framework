#pragma once
#include <seastar/core/future.hh>

namespace cpv {
	/** Contains actual read size and is end of stream */
	struct InputStreamReadResult {
		std::size_t size;
		bool eof;
		
		InputStreamReadResult(std::size_t sizeVal, bool eofVal) :
			size(sizeVal), eof(eofVal) { }
		InputStreamReadResult() : InputStreamReadResult(0, true) { }
	};
	
	/** Interface of simple input stream */
	class InputStreamBase {
	public:
		/** Virtual destructor */
		virtual ~InputStreamBase() = default;
		
		/** Read data from stream, the buffer must live until future resolved */
		virtual seastar::future<InputStreamReadResult> read(char* buf, std::size_t size) = 0;
	};
}

