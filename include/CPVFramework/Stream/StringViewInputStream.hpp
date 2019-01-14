#pragma once
#include <string_view>
#include "./InputStreamBase.hpp"

namespace cpv {
	/** Input stream that use given string view as data source */
	class StringViewInputStream : public InputStreamBase {
	public:
		/** Read data from stream, the buffer must live until future resolved */
		seastar::future<InputStreamReadResult> read(char* buf, std::size_t size) override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset(const std::string_view& str);
		
		/** Constructor */
		StringViewInputStream();
		
	private:
		std::string_view str_;
		std::size_t position_;
	};
}

