#pragma once
#include "./Exception.hpp"

namespace cpv {
	/**
	 * Deserialize error, usually mean the format of serialized value is incorrect
	 * Example: throw Deserialize(CPV_CODEINFO, "some error");
	 */
	class DeserializeException : public Exception {
	public:
		using Exception::Exception;
	};
}

