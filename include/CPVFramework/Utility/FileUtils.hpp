#pragma once
#include <string>
#include <string_view>

namespace cpv {
	/** A shortcut function to read file contents */
	std::string readFile(const std::string_view& filename);

	/** A shortcut function to write file contents */
	void writeFile(const std::string_view& filename, const std::string_view& contents);
}

