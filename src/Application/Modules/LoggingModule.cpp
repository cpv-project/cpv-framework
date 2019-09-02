#include <CPVFramework/Application/Modules/LoggingModule.hpp>

namespace cpv {
	/** Set custom logger */
	void LoggingModule::setLogger(const seastar::shared_ptr<Logger>& logger) {
		logger_ = logger;
	}

	/** Do some work for given application state */
	seastar::future<> LoggingModule::handle(Container& container, ApplicationState state) {
		if (state == ApplicationState::RegisterServices) {
			container.add<seastar::shared_ptr<Logger>>(logger_);
		}
		return seastar::make_ready_future<>();
	}

	/** Constructor */
	LoggingModule::LoggingModule() :
		logger_(Logger::createConsole(LogLevel::Notice)) { }
}

