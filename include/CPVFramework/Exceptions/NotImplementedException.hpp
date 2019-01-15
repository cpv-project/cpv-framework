#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Error means feature has not yet implemented.
	 * Example: throw NotImplementedException(CPV_CODEINFO, "some error");
	 */
	class NotImplementedException : public Exception {
	public:
		using Exception::Exception;
	};
}

