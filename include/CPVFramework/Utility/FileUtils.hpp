#pragma once
#include "./SharedString.hpp"

namespace cpv {
	/** A shortcut function to read file contents */
	SharedString readFile(std::string_view filename);

	/** A shortcut function to write file contents */
	void writeFile(std::string_view filename, std::string_view contents);

	/** Check whether filename is safe (not contains ".." or "//") */
	bool isSafePath(std::string_view filename);
}

