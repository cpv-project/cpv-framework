#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Data format error, usually cause by library user.
	 * Example: throw FormatException(CPV_CODEINFO, "some error");
	 */
	class FormatException : public Exception {
	public:
		using Exception::Exception;
	};
}

