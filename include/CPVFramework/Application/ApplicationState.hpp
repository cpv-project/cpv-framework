#pragma once
#include "../Utility/EnumUtils.hpp"

namespace cpv {
	/** State of an application, used to invoke ModuleBase::handle */
	enum class ApplicationState {
		/** The first state of initializaztion, application will only initialize once  */
		StartInitialize = 0,
		/** Register basic services that may used in custom initialize functions */
		RegisterBasicServices = 100,
		/** Before calling custom initialize functions that passed at module registration  */
		BeforeCallCustomInitializeFunctions = 101,
		/** This is a dummy state, it won't used to invoke to ModuleBase::handle */
		CallingCustomIntializeFunctions = 102,
		/** After custom initialize functions that passed at module registration called */
		AfterCustomInitializeFunctionsCalled = 103,
		/** Register services that order matters and wants to put first */
		RegisterHeadServices = 104,
		/** Register common services, order of services dependents on order of moduels */
		RegisterServices = 105,
		/** Register services that order matters and wants to put last */
		RegisterTailServices = 106,
		/** Patch services that already registered */
		PatchServices = 107,
		/** After all services registered */
		AfterServicesRegistered = 199,
		/** The last state of initializaztion */
		AfterInitialized = 999,

		/** The first state when Application::start invoked */
		BeforeStart = 10000,
		/** The second state when Application::start invoked */
		Starting = 10001,
		/** The last state when Application::start invoked */
		AfterStarted = 10099,

		/** The first state when Application::stopTemporary invoked */
		BeforeTemporaryStop = 10100,
		/** The second state when Application::stopTemporary invoked */
		TemporaryStopping = 10101,
		/** The last state when Application::stopTemporary invoked */
		AfterTemporaryStopped = 10199,

		/** The first state when Application::stop invoked */
		BeforeStop = 10200,
		/** The second state when Application::stop invoked */
		Stopping = 10201,
		/** The last state when Application::stop invoked */
		AfterStopped = 10299,
	};

	/** Enum descriptions of ApplicationState */
	template <>
	struct EnumDescriptions<ApplicationState> {
		static const std::vector<std::pair<ApplicationState, const char*>>& get();
	};
}

