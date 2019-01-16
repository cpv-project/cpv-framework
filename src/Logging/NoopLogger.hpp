#include <CPVFramework/Logging/Logger.hpp>

namespace cpv {
	/** A do-nothing logger */
	class NoopLogger : public Logger {
	public:
		/** Constructor */
		NoopLogger();

	protected:
		/** The implmentation of log, do nothing */
		void logImpl(LogLevel, const std::string&) override;
	};
}

