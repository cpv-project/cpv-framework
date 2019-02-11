#pragma once
#include <string_view>
#include <seastar/core/future.hh>
#include <seastar/core/temporary_buffer.hh>

namespace cpv {
	/** Contains data read from stream */
	struct InputStreamReadResult {
		/** The storage of data, if it's empty that mean the storage is inside stream */
		seastar::temporary_buffer<char> underlyingBuffer;
		/** Data read from stream */
		std::string_view data;
		/** Whether data is the last part of stream */
		bool isEnd;
		
		InputStreamReadResult(
			seastar::temporary_buffer<char>&& underlyingBufferVal,
			const std::string_view& dataVal,
			bool isEndVal) :
			underlyingBuffer(std::move(underlyingBufferVal)), data(dataVal), isEnd(isEndVal) { }
		InputStreamReadResult(const std::string_view& dataVal, bool isEndVal) :
			InputStreamReadResult({}, dataVal, isEndVal) { }
		InputStreamReadResult() : InputStreamReadResult({}, {}, true) { }
	};
	
	/** Interface of simple input stream */
	class InputStreamBase {
	public:
		/** Virtual destructor */
		virtual ~InputStreamBase() = default;
		
		/** Read data from stream */
		virtual seastar::future<InputStreamReadResult> read() = 0;
	};
}

