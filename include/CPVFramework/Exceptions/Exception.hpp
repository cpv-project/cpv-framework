#pragma once
#include <stdexcept>
#include "../Utility/StringUtils.hpp"
#include "../Utility/CodeInfo.hpp"

namespace cpv {
	/**
	 * The base class of all exceptions this library will throw.
	 * Example: throw Exception(CPV_CODEINFO, "some error");
	 */
	class Exception : public std::runtime_error {
	public:
		/** Constructor */
		template <class... Args>
		Exception(CodeInfo&& codeInfo, Args&&... args) :
			Exception(std::move(codeInfo).str(), joinString(" ", std::forward<Args>(args)...)) { }
		
	protected:
		/** Constructor */
		Exception(std::string&& codeInfoStr, std::string&& message);
	};
}

