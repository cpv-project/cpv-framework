#pragma once
#include "./InputStreamBase.hpp"

namespace cpv {
	/** Input stream that use given string as data source */
	class StringInputStream : public InputStreamBase {
	public:
		/** Read data from stream */
		seastar::future<InputStreamReadResult> read() override;
		
		/** For Object<> */
		void freeResources();
		
		/** For Object<> */
		void reset(std::string&& str);
		
		/** Constructor */
		StringInputStream();
		
	private:
		std::string str_;
		bool isEnd_;
	};
}

