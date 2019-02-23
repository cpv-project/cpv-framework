#pragma once
#include <string_view>
#include <optional>
#include <seastar/core/future.hh>
#include <seastar/core/temporary_buffer.hh>

namespace cpv {
	/** Contains data read from stream */
	struct InputStreamReadResult {
		/** Data read from stream */
		seastar::temporary_buffer<char> data;
		/** Whether data is the last part of stream */
		bool isEnd;
		
		/** Get string view of the data read from stream */
		std::string_view view() const { return std::string_view(data.get(), data.size()); }
		
		/** Constructor */
		InputStreamReadResult(seastar::temporary_buffer<char>&& dataVal, bool isEndVal) :
			data(std::move(dataVal)), isEnd(isEndVal) { }
		
		/** Constructor */
		InputStreamReadResult() : InputStreamReadResult({}, true) { }
	};
	
	/**
	 * Interface of simple input stream.
	 * The read function will return a mutable buffer contains data.
	 * The mutable buffer is useful for pass to an inplace (in-situ) parser.
	 * Seek is not supported so it's easy to implement and with less overhead.
	 */
	class InputStreamBase {
	public:
		/** Virtual destructor */
		virtual ~InputStreamBase() = default;
		
		/** Read data from stream */
		virtual seastar::future<InputStreamReadResult> read() = 0;
		
		/** Get the total size of stream, may return empty if not supported */
		virtual std::optional<std::size_t> size() const { return { }; }
	};
}

