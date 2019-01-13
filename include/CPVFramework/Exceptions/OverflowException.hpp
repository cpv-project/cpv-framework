#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Error for arithmetic overflow or underflow
	 * Example: throw OverflowException(CPV_CODEINFO, "some error");
	 */
	class OverflowException : public Exception {
	public:
		using Exception::Exception;
	};
}

