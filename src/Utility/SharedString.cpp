#include <CPVFramework/Utility/SharedString.hpp>
#include <CPVFramework/Utility/SharedStringBuilder.hpp>

namespace cpv {
	/** Construct with string representation of integer */
	template <class CharType>
	BasicSharedString<CharType> BasicSharedString<CharType>::fromIntImpl(std::intmax_t value) {
		return BasicSharedStringBuilder<CharType>().append(value).build();
	}

	/** Construct with string representation of integer */
	template <class CharType>
	BasicSharedString<CharType> BasicSharedString<CharType>::fromIntImpl(std::uintmax_t value) {
		return BasicSharedStringBuilder<CharType>().append(value).build();
	}

	/** Construct with string representation of floating point */
	template <class CharType>
	BasicSharedString<CharType> BasicSharedString<CharType>::fromDoubleImpl(double value) {
		return BasicSharedStringBuilder<CharType>().append(value).build();
	}

	/** Construct with string representation of floating point */
	template <class CharType>
	BasicSharedString<CharType> BasicSharedString<CharType>::fromDoubleImpl(long double value) {
		return BasicSharedStringBuilder<CharType>().append(value).build();
	}

	// Template instatiations
	template BasicSharedString<char> BasicSharedString<char>::fromIntImpl(std::intmax_t);
	template BasicSharedString<char> BasicSharedString<char>::fromIntImpl(std::uintmax_t);
	template BasicSharedString<char> BasicSharedString<char>::fromDoubleImpl(double);
	template BasicSharedString<char> BasicSharedString<char>::fromDoubleImpl(long double);
}

