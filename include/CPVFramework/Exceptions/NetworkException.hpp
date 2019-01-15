#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Network related exception like connect, send or receive error.
	 * Example: throw NetworkException(CPV_CODEINFO, "some error");
	 */
	class NetworkException : public Exception {
	public:
		using Exception::Exception;
	};
}

