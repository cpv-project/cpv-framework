#include <CPVFramework/Module/DefaultModule.hpp>
#include <CPVFramework/Utility/JsonUtils.hpp>
#include <CPVFramework/Utility/EnumUtils.hpp>
#include <CPVFramework/Logging/Logger.hpp>

namespace cpv {
	/** Register default services */
	seastar::future<> DefaultModule::registerServices(Container& container) {
		// add console logger
		auto logLevelStr = configuration_->value<std::string>("logging.log_level", "Info");
		auto logLevel = enumFromString<LogLevel>(logLevelStr, LogLevel::Info);
		container.add<seastar::shared_ptr<Logger>>(Logger::createConsole(logLevel));
		return seastar::make_ready_future<>();
	}
}

