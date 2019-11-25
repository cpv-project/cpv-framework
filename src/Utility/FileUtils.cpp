#include <fstream>
#include <streambuf>
#include <cstring>
#include <CPVFramework/Utility/FileUtils.hpp>
#include <CPVFramework/Utility/Macros.hpp>
#include <CPVFramework/Exceptions/FileSystemException.hpp>

namespace cpv {
	/** A shortcut function to read file contents */
	SharedString readFile(std::string_view filename) {
		std::ifstream file(
			std::string(filename), // replace to std::filesystem::u8path?
			std::ios::in | std::ios::binary);
		if (CPV_UNLIKELY(!file.is_open())) {
			throw FileSystemException(CPV_CODEINFO,
				"open file", filename, "failed:", std::strerror(errno));
		} else {
			file.seekg(0, std::ios::end);
			std::size_t size = file.tellg();
			SharedString str(size);
			file.seekg(0);
			file.read(str.data(), size);
			return str;
		}
	}

	/** A shortcut function to write file contents */
	void writeFile(std::string_view filename, std::string_view contents) {
		std::ofstream file(
			std::string(filename), // replace to std::filesystem::u8path?
			std::ios::out | std::ios::binary);
		if (CPV_UNLIKELY(!file.is_open())) {
			throw FileSystemException(CPV_CODEINFO,
				"open file", filename, "failed:", std::strerror(errno));
		} else {
			file.write(contents.data(), contents.size());
			if (CPV_UNLIKELY(!file)) {
				throw FileSystemException(CPV_CODEINFO,
					"write file", filename, "failed:", std::strerror(errno));
			}
		}
	}

	/** Check whether filename is safe (not contains ".." or "//") */
	bool isSafePath(std::string_view filename) {
		char lastChar = 0;
		for (char c : filename) {
			if (CPV_UNLIKELY(
				c == '\r' || c == '\n' || c == '\x00' ||
				(c == '.' && lastChar == '.') ||
				(c == '/' && lastChar == '/') ||
				(c == '\\' && lastChar == '\\'))) {
				return false;
			}
			lastChar = c;
		}
		return true;
	}
}

