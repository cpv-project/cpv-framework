#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Arithmetic overflow or underflow exception.
	 * Example: throw OverflowException(CPV_CODEINFO, "some error");
	 */
	class OverflowException : public Exception {
	public:
		using Exception::Exception;
	};
}

