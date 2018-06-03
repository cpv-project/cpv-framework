#include <CPVFramework/Application/ApplicationControl.hpp>
#include <CPVFramework/Http/Handler.hpp>
#include <CPVFramework/Utility/JsonUtils.hpp>
#include <TestUtility/GTestUtils.hpp>

namespace {
	struct Base {
		virtual std::size_t getValue() = 0;
		virtual void setValue(std::size_t value) = 0;
	};

	struct Derived : public Base {
		std::size_t getValue() override { return value_; }
		void setValue(std::size_t value) override { value_ = value; }
	private:
		std::size_t value_;
	};

	struct ModuleA : public cpv::Module {
		using Module::Module;
		
		seastar::future<> registerServices(cpv::Container& container) override {
			container.add<seastar::shared_ptr<Base>, seastar::shared_ptr<Derived>>(cpv::Lifetime::Singleton);
			return seastar::sleep(std::chrono::milliseconds(1));
		}

		seastar::future<> initializeServices(const cpv::Container& container) override {
			container.get<seastar::shared_ptr<Base>>()->setValue(123);
			return seastar::make_ready_future<>();
		};
	};

	struct ModuleB : public cpv::Module {
		using Module::Module;

		seastar::future<> registerRoutes(
			const seastar::shared_ptr<const cpv::Container>&,
			cpv::httpd::http_server& server) override {
			server._routes.put(
				cpv::httpd::operation_type::GET,
				"/module_b",
				new cpv::httpd::function_handler([] (cpv::httpd::const_req) {
					return "module b action result";
				}));
			return seastar::make_ready_future<>();
		}
	};

	class MyApplication : public cpv::Application {
	public:
		using Application::Application;

		seastar::future<> registerModules() override {
			registerModule<ModuleA>();
			registerModule<ModuleB>();
			return seastar::make_ready_future<>();
		}

		seastar::future<> verify() {
			return seastar::make_ready_future<>().then([this] {
				ASSERT_EQ(container_->get<seastar::shared_ptr<Base>>()->getValue(), 123);
			});
		}
	};

	static const inline std::string configuration(cpv::joinString("", "{",
		"\"httpd.listen_hostname\": \"", HTTPD_LISTEN_IP, "\",",
		"\"httpd.listen_port\": ", HTTPD_LISTEN_PORT, ",",
		"\"logging.log_level\": \"Error\"",
		"}"));
}

TEST_FUTURE(TestApplication, start) {
	cpv::ApplicationControl<MyApplication> applicationControl;
	return applicationControl.start(configuration).then([applicationControl] {
		return applicationControl.stop();
	});
}

TEST_FUTURE(TestApplication, invokeOnAll) {
	cpv::ApplicationControl<MyApplication> applicationControl;
	return applicationControl.start(configuration).then([applicationControl] {
		return applicationControl.invokeOnAll([] (MyApplication& application) {
			return application.verify();
		});
	}).then([applicationControl] {
		return applicationControl.stop();
	});
}

