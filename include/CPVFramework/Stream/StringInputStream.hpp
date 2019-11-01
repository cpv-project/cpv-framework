#pragma once
#include "./InputStreamBase.hpp"

namespace cpv {
	/** Input stream that use given string as data source */
	class StringInputStream : public InputStreamBase {
	public:
		/** Read data from stream, notice the lifetime of data is bound to stream */
		seastar::future<InputStreamReadResult> read() override;
		
		/** Get the hint of total size of stream */
		std::optional<std::size_t> sizeHint() const override;
		
		/** For Reusable<> */
		void freeResources();
		
		/** For Reusable<> */
		void reset(std::string&& str);
		
		/** Constructor */
		StringInputStream();
		
	private:
		std::string str_;
		bool isEnd_;
	};
}

