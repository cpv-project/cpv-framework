#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * File system related exception like open file error
	 * Example: throw FileSystemException(CPV_CODEINFO, "some error");
	 */
	class FileSystemException : public Exception {
	public:
		using Exception::Exception;
	};
}

