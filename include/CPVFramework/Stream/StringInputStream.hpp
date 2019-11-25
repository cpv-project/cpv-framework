#pragma once
#include "../Utility/SharedString.hpp"
#include "./InputStreamBase.hpp"

namespace cpv {
	/** Input stream that use given string as data source */
	class StringInputStream : public InputStreamBase {
	public:
		/** Read data from stream */
		seastar::future<InputStreamReadResult> read() override;
		
		/** Get the hint of total size of stream */
		std::optional<std::size_t> sizeHint() const override;
		
		/** For Reusable<> */
		void freeResources();
		
		/** For Reusable<> */
		void reset(SharedString&& str);
		
		/** Constructor */
		StringInputStream();
		
	private:
		SharedString str_;
		std::size_t sizeHint_;
	};
}

