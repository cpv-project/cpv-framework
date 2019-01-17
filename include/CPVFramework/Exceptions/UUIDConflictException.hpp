#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Exception throws when two uuid is conflict.
	 * Example: throw UUIDConflictException(CPV_CODEINFO, "some error");
	 */
	class UUIDConflictException : public Exception {
	public:
		using Exception::Exception;
	};
}

