#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Logic error, usually mean there something wrong in the code
	 * Example: throw LogicException(CPV_CODEINFO, "some error");
	 */
	class LogicException : public Exception {
	public:
		using Exception::Exception;
	};
}

