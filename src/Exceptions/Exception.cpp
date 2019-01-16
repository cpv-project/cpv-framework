#include <execinfo.h>
#include <array>
#include <memory>
#include <functional>
#include <CPVFramework/Exceptions/Exception.hpp>

namespace cpv {
	namespace {
		/** Build full exception message with stack trace information */
		std::string buildExceptionMessage(std::string&& codeInfoStr, std::string&& message) {
			static thread_local std::array<void*, 1024> frames;
			// append code location and original message
			std::string fullMessage;
			fullMessage.append(codeInfoStr).append(" ").append(message).append("\n");
			// append backtrace information
			fullMessage.append("backtrace:\n");
			int size = ::backtrace(frames.data(), frames.size());
			if (size < 0 || static_cast<std::size_t>(size) >= frames.size()) {
				fullMessage.append("  call backtrace failed\n");
				return fullMessage;
			}
			std::unique_ptr<char*[], std::function<void(char*[])>> symbols(
				::backtrace_symbols(frames.data(), size), std::free);
			if (symbols == nullptr) {
				fullMessage.append("  call backtrace_symbols failed\n");
				return fullMessage;
			}
			for (int i = 0; i < size; ++i) {
				fullMessage.append("  ").append(symbols[i]).append("\n");
			}
			return fullMessage;
		}
	}
	
	/** Constructor */
	Exception::Exception(std::string&& codeInfoStr, std::string&& message) :
		std::runtime_error(buildExceptionMessage(std::move(codeInfoStr), std::move(message))) { }
}

