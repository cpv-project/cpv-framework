#include <chrono>
#include <vector>
#include <memory>
#include <seastar/core/future-util.hh>
#include <seastar/core/sleep.hh>
#include <seastar/core/sharded.hh>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Application/Application.hpp>

namespace cpv {
	/** ApplicationData for per core */
	class ApplicationPerCoreData {
	public:
		/** For seastar::sharded */
		seastar::future<> stop() {
			return seastar::make_ready_future<>();
		}

		/** Constructor */
		ApplicationPerCoreData() :
			container(),
			modules() { }

	public:
		Container container;
		std::vector<std::unique_ptr<ModuleBase>> modules;
	};

	/** Members of Application */
	class ApplicationData {
	public:
		/** Constructor */
		ApplicationData() :
			state(ApplicationState::StartInitialize),
			modulesFactories(),
			shardData_() { }

		/** Create sharedData with nullptr checking */
		std::unique_ptr<seastar::sharded<ApplicationPerCoreData>>& createShardData() & {
			if (shardData_ != nullptr) {
				throw LogicException(CPV_CODEINFO, "shardData already created");
			}
			shardData_ = std::make_unique<seastar::sharded<ApplicationPerCoreData>>();
			return shardData_;
		}

		/** Get sharedData with nullptr checking */
		std::unique_ptr<seastar::sharded<ApplicationPerCoreData>>& getShardData() & {
			if (shardData_ == nullptr) {
				throw LogicException(CPV_CODEINFO,
					"application already stopped permanently");
			}
			return shardData_;
		}

		/** destruct modules on all cpu cores */
		seastar::future<> stop() {
			return seastar::do_with(std::move(getShardData()), [] (auto& shardData) {
				return shardData->stop();
			});
		}

		/** Destructor */
		~ApplicationData() {
			if (shardData_ != nullptr) {
				std::cerr << "Please call Application::stop before exit program" << std::endl;
				std::terminate();
			}
		}

	public:
		ApplicationState state;
		std::vector<std::pair<
			std::function<std::unique_ptr<ModuleBase>()>,
			std::function<void(ModuleBase*)>>> modulesFactories;

	private:
		std::unique_ptr<seastar::sharded<ApplicationPerCoreData>> shardData_;
	};

	namespace {
		/** Update state and invoke handle function of modules on all cpu cores */
		seastar::future<> updateState(ApplicationData& data, ApplicationState state) {
			data.state = state;
			return data.getShardData()->invoke_on_all(
				[state] (ApplicationPerCoreData& perCoreData) {
				return seastar::do_for_each(perCoreData.modules,
					[state, &perCoreData] (std::unique_ptr<ModuleBase>& module) {
					return module->handle(perCoreData.container, state);
				});
			});
		}

		/** Ensure modules on all cpu cores are created and initialized */
		seastar::future<> ensureInitialized(ApplicationData& data) {
			if (data.state != ApplicationState::StartInitialize) {
				return seastar::make_ready_future<>();
			}
			return data.createShardData()->start().then([&data] {
				// create modules on all cpu cores
				return data.getShardData()->invoke_on_all(
					[&data] (ApplicationPerCoreData& perCoreData) {
					for (const auto& pair : data.modulesFactories) {
						perCoreData.modules.emplace_back(pair.first());
					}
				});
			}).then([&data] {
				return updateState(data, ApplicationState::StartInitialize);
			}).then([&data] {
				return updateState(data, ApplicationState::RegisterBasicServices);
			}).then([&data] {
				return updateState(data,
					ApplicationState::BeforeCallCustomInitializeFunctions);
			}).then([&data] {
				// call custom initalize functions
				data.state = ApplicationState::CallingCustomIntializeFunctions;
				return data.getShardData()->invoke_on_all(
					[&data] (ApplicationPerCoreData& perCoreData) {
					std::size_t moduleCount = perCoreData.modules.size();
					if (data.modulesFactories.size() != moduleCount) {
						throw LogicException(CPV_CODEINFO, "module count not matched");
					}
					for (std::size_t i = 0; i < moduleCount; ++i) {
						auto& func = data.modulesFactories[i].second;
						if (func != nullptr) {
							func(perCoreData.modules[i].get());
						}
					}
				});
			}).then([&data] {
				return updateState(data,
					ApplicationState::AfterCustomInitializeFunctionsCalled);
			}).then([&data] {
				return updateState(data, ApplicationState::RegisterHeadServices);
			}).then([&data] {
				return updateState(data, ApplicationState::RegisterServices);
			}).then([&data] {
				return updateState(data, ApplicationState::RegisterTailServices);
			}).then([&data] {
				return updateState(data, ApplicationState::PatchServices);
			}).then([&data] {
				return updateState(data, ApplicationState::AfterServicesRegistered);
			}).then([&data] {
				return updateState(data, ApplicationState::AfterInitialized);
			});
		}

		/** Start application, invoke handle function of modules on all cpu cores */
		seastar::future<> startImpl(ApplicationData& data) {
			if (data.state != ApplicationState::AfterInitialized &&
				data.state != ApplicationState::AfterTemporaryStopped) {
				return seastar::make_exception_future<>(LogicException(
					CPV_CODEINFO,
					"incorrect state when starting application:", data.state));
			}
			return updateState(data, ApplicationState::BeforeStart).then([&data] {
				return updateState(data, ApplicationState::Starting);
			}).then([&data] {
				return updateState(data, ApplicationState::AfterStarted);
			});
		}

		/** Stop application temporary, invoke handle function of modules on all cpu cores */
		seastar::future<> stopTemporaryImpl(ApplicationData& data) {
			if (data.state != ApplicationState::AfterStarted) {
				return seastar::make_exception_future<>(LogicException(
					CPV_CODEINFO,
					"incorrect state when temporary stopping application:", data.state));
			}
			return updateState(data, ApplicationState::BeforeTemporaryStop).then([&data] {
				return updateState(data, ApplicationState::TemporaryStopping);
			}).then([&data] {
				return updateState(data, ApplicationState::AfterTemporaryStopped);
			});
		}

		/** Stop application permanently, invoke handle function of modules on all cpu cores */
		seastar::future<> stopImpl(ApplicationData& data) {
			if (data.state != ApplicationState::AfterStarted &&
				data.state != ApplicationState::AfterTemporaryStopped) {
				return seastar::make_exception_future<>(LogicException(
					CPV_CODEINFO,
					"incorrect state when stopping application:", data.state));
			}
			return updateState(data, ApplicationState::BeforeStop).then([&data] {
				return updateState(data, ApplicationState::Stopping);
			}).then([&data] {
				return updateState(data, ApplicationState::AfterStopped);
			}).then([&data] {
				return data.stop();
			});
		}
	}

	/** Start application */
	seastar::future<> Application::start() {
		auto data = data_;
		return seastar::do_with(std::move(data), [] (auto& data) {
			return ensureInitialized(*data).then([&data] {
				return startImpl(*data);
			});
		});
	}

	/** Stop application temporary */
	seastar::future<> Application::stopTemporary() {
		auto data = data_;
		return seastar::do_with(std::move(data), [] (auto& data) {
			return stopTemporaryImpl(*data);
		});
	}

	/** Stop application permanently */
	seastar::future<> Application::stop() {
		auto data = data_;
		return seastar::do_with(std::move(data), [] (auto& data) {
			return stopImpl(*data);
		});
	}

	/** Run application until program exit (e.g. Ctrl+C) */
	seastar::future<> Application::runForever() {
		auto data = data_;
		return seastar::do_with(
			std::move(data), seastar::make_shared<std::atomic_bool>(),
			[](auto& data, auto& stop) {
			// register atexit function for Ctrl+C support
			seastar::engine().at_exit([stop] {
				stop->store(true);
				return seastar::make_ready_future();
			});
			// ensure initialized
			return ensureInitialized(*data).then([&data] {
				// start application
				return startImpl(*data);
			}).then([&data, &stop] {
				// wait until stop flag set to true
				return seastar::do_until(
					[&stop] { return stop->load(); },
					[] { return seastar::sleep(std::chrono::seconds(1)); });
			}).then([&data] {
				// stop application
				return stopImpl(*data);
			}).then([] {
				// wait for asynchronously cleanup to make leak sanitizer happy
				return seastar::sleep(std::chrono::seconds(1));
			});
		});
	}

	/** Constructor */
	Application::Application() :
		data_(seastar::make_shared<ApplicationData>()) { }

	/** Add module to application (implementation) */
	void Application::add(
		const std::function<std::unique_ptr<ModuleBase>()>& moduleFactory,
		const std::function<void(ModuleBase*)>& initializeFunction) {
		if (CPV_UNLIKELY(data_->state != ApplicationState::StartInitialize)) {
			throw LogicException(CPV_CODEINFO,
				"Can't add module after application initialized (atleast stated once)");
		}
		data_->modulesFactories.emplace_back(moduleFactory, initializeFunction);
	}
}

