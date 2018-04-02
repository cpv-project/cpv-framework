#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * The feature has not yet implemented
	 * Example: throw NotImplementedException(CPV_CODEINFO, "some error");
	 */
	class NotImplementedException : public Exception {
	public:
		using Exception::Exception;
	};
}

