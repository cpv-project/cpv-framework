#pragma once
#include <iostream>
#include "./StringUtils.hpp"

// convenient macro to tell where is the line that includes this code
// since __func__, __FUNC__, __PRETTY_FUNCTION isn't macro so a helper function is required
#define CPV_CODEINFO cpv::CodeInfo(cpv::joinString("", "[", __FILE__, ":", __LINE__, ":", __PRETTY_FUNCTION__, "]"))

namespace cpv {
	/** Code information wrapper class, see macro CPV_CODEINFO */
	class CodeInfo {
	public:
		/** Get the string inside this class */
		std::string str() const& { return str_; }
		std::string str() && { return std::move(str_); }

		/** Constructor */
		explicit CodeInfo(std::string&& str) :
			str_(std::move(str)) { }

	private:
		std::string str_;
	};
}

