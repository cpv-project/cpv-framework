#include <seastar/core/app-template.hh>
#include <seastar/core/sleep.hh>
#include <CPVFramework/Application/ApplicationControl.hpp>
#include <CPVFramework/Module/PrometheusModule.hpp>
#include <CPVFramework/Container/Injector.hpp>
#include <CPVFramework/Logging/Logger.hpp>
#include <CPVFramework/Web/ServiceHandler.hpp>

namespace {
	static const inline std::string configuration = R"({
		"httpd.listen_hostname": "127.0.0.1",
		"httpd.listen_port": 8001,
		"logging.log_level": "Debug"
	})";

	class HelloService {
	public:
		seastar::sstring replyText(cpv::httpd::const_req) {
			logger_->log(cpv::LogLevel::Debug, "reply text");
			return "hello world!";
		}

		seastar::future<cpv::Json> replyJson(cpv::httpd::const_req) {
			return seastar::sleep(std::chrono::seconds(1)).then([this] {
				logger_->log(cpv::LogLevel::Debug, "reply json");
				cpv::Json result;
				result["abc"] = 123;
				return result;
			});
		}

		explicit HelloService(const seastar::shared_ptr<cpv::Logger>& logger) :
			logger_(logger) {
			logger_->log(cpv::LogLevel::Debug, "construct hello service");
		}

	private:
		seastar::shared_ptr<cpv::Logger> logger_;
	};

	class HelloModule : public cpv::Module {
	public:
		using Module::Module;

		seastar::future<> registerServices(cpv::Container& container) override {
			container.add<HelloService,
				cpv::Injector<HelloService, seastar::shared_ptr<cpv::Logger>>>();
			return seastar::make_ready_future<>();
		}

		seastar::future<> registerRoutes(
			const seastar::shared_ptr<const cpv::Container>& container,
			cpv::httpd::http_server& server) override {
			server._routes.put(
				cpv::httpd::operation_type::GET,
				"/",
				new cpv::ServiceHandler<HelloService, &HelloService::replyText>(container));
			server._routes.put(
				cpv::httpd::operation_type::GET,
				"/json",
				new cpv::ServiceHandler<HelloService, &HelloService::replyJson>(container));
			return seastar::make_ready_future<>();
		}
	};

	class HelloApplication : public cpv::Application {
	public:
		using Application::Application;

		seastar::future<> registerModules() override {
			registerModule<cpv::PrometheusModule>();
			registerModule<HelloModule>();
			return seastar::make_ready_future<>();
		}
	};
}

int main(int argc, char** argv) {
	seastar::app_template app;
	app.run(argc, argv, [] {
		cpv::ApplicationControl<HelloApplication> applicationControl;
		return applicationControl.start(configuration).then([applicationControl] {
			return applicationControl.wait();
		}).then([applicationControl] {
			return applicationControl.stop();
		});
	});
	return 0;
}

