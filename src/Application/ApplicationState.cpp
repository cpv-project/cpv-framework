#include <CPVFramework/Application/ApplicationState.hpp>

namespace cpv {
	/** Enum descriptions of ApplicationState */
	const std::vector<std::pair<ApplicationState, const char*>>&
		EnumDescriptions<ApplicationState>::get() {
		static std::vector<std::pair<ApplicationState, const char*>> staticNames({
			{ ApplicationState::StartInitialize, "StartInitialize" },
			{ ApplicationState::RegisterBasicServices, "RegisterBasicServices" },
			{ ApplicationState::BeforeCallCustomInitializeFunctions, "BeforeCallCustomInitializeFunctions" },
			{ ApplicationState::CallingCustomIntializeFunctions, "CallingCustomIntializeFunctions" },
			{ ApplicationState::AfterCustomInitializeFunctionsCalled, "AfterCustomInitializeFunctionsCalled" },
			{ ApplicationState::RegisterHeadServices, "RegisterHeadServices" },
			{ ApplicationState::RegisterServices, "RegisterServices" },
			{ ApplicationState::RegisterTailServices, "RegisterTailServices" },
			{ ApplicationState::PatchServices, "PatchServices" },
			{ ApplicationState::AfterServicesRegistered, "AfterServicesRegistered" },
			{ ApplicationState::AfterInitialized, "AfterInitialized" },
			{ ApplicationState::BeforeStart, "BeforeStart" },
			{ ApplicationState::Starting, "Starting" },
			{ ApplicationState::AfterStarted, "AfterStarted" },
			{ ApplicationState::BeforeStop, "BeforeStop" },
			{ ApplicationState::Stopping, "Stopping" },
			{ ApplicationState::AfterStopped, "AfterStopped" }
		});
		return staticNames;
	}
}

