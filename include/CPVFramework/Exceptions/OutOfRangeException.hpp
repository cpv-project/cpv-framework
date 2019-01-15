#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Error means access by index out of range or by key not exists.
	 * Example: throw OutOfRangeException(CPV_CODEINFO, "some error");
	 */
	class OutOfRangeException : public Exception {
	public:
		using Exception::Exception;
	};
}

