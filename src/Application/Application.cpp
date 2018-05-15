#include <core/sleep.hh>
#include <core/prometheus.hh>
#include <CPVFramework/Application/Application.hpp>
#include <CPVFramework/Utility/FileUtils.hpp>
#include <CPVFramework/Utility/JsonUtils.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>

namespace cpv {
	namespace {
		std::string generateServerName() {
			static thread_local std::size_t id;
			return joinString("", "cpv-httpd-", id++);
		}
	}

	/** Start application */
	seastar::future<> Application::start() {
		if (started_) {
			throw LogicException(CPV_CODEINFO, "application already started");
		}
		started_ = true;
		return registerModules().then([this] {
			return seastar::do_for_each(modules_.begin(), modules_.end(), [this] (auto& module) {
				return module->registerServices(*container_);
			});
		}).then([this] {
			return seastar::do_for_each(modules_.begin(), modules_.end(), [this] (auto& module) {
				return module->initializeServices(*container_);
			});
		}).then([this] {
			return seastar::do_for_each(modules_.begin(), modules_.end(), [this] (auto& module) {
				return module->registerRoutes(*server_);
			});
		}).then([this] {
			auto hostname = configuration_->value<std::string>("httpd.listen_hostname", "127.0.0.1");
			auto port = configuration_->value<std::uint16_t>("httpd.listen_port", 8000);
			return server_->listen({ hostname, port });
		});
	}

	/** Stop application */
	seastar::future<> Application::stop() {
		return server_->stop();
	}

	/** Constructor */
	Application::Application() :
		Application("{}") { }

	/** Constructor */
	Application::Application(const std::string& configurationJson) :
		configuration_(seastar::make_shared<const Json>(Json::parse(configurationJson))),
		container_(Container::create()),
		server_(seastar::make_shared<httpd::http_server>(generateServerName())),
		modules_(),
		started_(false) { }
}

