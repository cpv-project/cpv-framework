#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Error means buffer length not enough.
	 * Example: throw LengthException(CPV_CODEINFO, "some error");
	 */
	class LengthException : public Exception {
	public:
		using Exception::Exception;
	};
}

