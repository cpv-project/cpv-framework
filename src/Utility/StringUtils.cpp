#include <CPVFramework/Utility/StringUtils.hpp>

namespace cpv {
	/** Compare two string case insensitive */
	bool caseInsensitiveEquals(std::string_view a, std::string_view b) {
		std::size_t size = a.size();
		if (size != b.size()) {
			return false;
		}
		for (std::size_t i = 0; i < size; ++i) {
			if (std::tolower(a[i]) != std::tolower(b[i])) {
				return false;
			}
		}
		return true;
	}
}

