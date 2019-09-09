#pragma once
#include <string>
#include <string_view>

namespace cpv {
	/** A shortcut function to read file contents */
	std::string readFile(std::string_view filename);

	/** A shortcut function to write file contents */
	void writeFile(std::string_view filename, std::string_view contents);
}

