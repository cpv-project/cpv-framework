#pragma once
#include "./InputStreamBase.hpp"

namespace cpv {
	/** Input stream that use given string as data source */
	class StringInputStream : public InputStreamBase {
	public:
		/** Read data from stream, the buffer must live until future resolved */
		seastar::future<InputStreamReadResult> read(char* buf, std::size_t size) override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset(std::string&& str);
		
		/** Constructor */
		StringInputStream();
		
	private:
		std::string str_;
		std::size_t position_;
	};
}

