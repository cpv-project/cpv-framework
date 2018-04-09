#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Container related exception like get service instance failed.
	 * Example: throw ContainerException(CPV_CODEINFO, "some error");
	 */
	class ContainerException : public Exception {
	public:
		using Exception::Exception;
	};
}

