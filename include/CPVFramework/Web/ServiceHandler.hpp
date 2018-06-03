#pragma once
#include "../Http/Handler.hpp"
#include "../Container/Container.hpp"
#include "../Utility/JsonUtils.hpp"

namespace cpv {
	/** Base class of ServiceHandler variants */
	class ServiceHandlerBase : public httpd::handler_base {
	public:
		ServiceHandlerBase(const seastar::shared_ptr<const Container>& container) :
			container_(container) { }

	protected:
		seastar::shared_ptr<const Container> container_;
	};

	/** Resolve service from container and use it to handle http request */
	template <class TService, auto Func>
	class ServiceHandler : public ServiceHandlerBase {
	public:
		using ServiceHandlerBase::ServiceHandlerBase;

		seastar::future<std::unique_ptr<httpd::reply>> handle(
			const seastar::sstring& path,
			std::unique_ptr<httpd::request> request,
			std::unique_ptr<httpd::reply> reply) override = delete;
	};

	/** For seastar::sstring(httpd::const_req) */
	template <class TService,
		seastar::sstring(ServiceAccessorTrait<TService>::ServiceType::*Func)(httpd::const_req)>
	class ServiceHandler<TService, Func> : public ServiceHandlerBase {
	public:
		using ServiceHandlerBase::ServiceHandlerBase;

		seastar::future<std::unique_ptr<httpd::reply>> handle(
			const seastar::sstring& path,
			std::unique_ptr<httpd::request> request,
			std::unique_ptr<httpd::reply> reply) override {
			auto instance = container_->get<TService>();
			reply->_content = (ServiceAccessorTrait<TService>()(instance).*(Func))(*request);
			reply->set_mime_type("text/plain");
			reply->done();
			return seastar::make_ready_future<std::unique_ptr<httpd::reply>>(std::move(reply));
		}
	};

	/** For seastar::future<Json>(httpd::const_req) */
	template <class TService,
		seastar::future<Json>(ServiceAccessorTrait<TService>::ServiceType::*Func)(httpd::const_req)>
	class ServiceHandler<TService, Func> : public ServiceHandlerBase {
	public:
		using ServiceHandlerBase::ServiceHandlerBase;

		seastar::future<std::unique_ptr<httpd::reply>> handle(
			const seastar::sstring& path,
			std::unique_ptr<httpd::request> request,
			std::unique_ptr<httpd::reply> reply) override {
			return seastar::do_with(
				container_->get<TService>(),
				std::move(request),
				std::move(reply),
				[](auto& instance, auto& request, auto& reply) {
				return (ServiceAccessorTrait<TService>()(instance).*(Func))(*request)
				.then([&reply] (Json json) {
					reply->_content = json.dump();
					reply->set_mime_type("application/json");
					reply->done();
					return std::move(reply);
				});
			});
		}
	};

	/** For seastar::future<>(httpd::const_req, httpd::reply&) */
	template <class TService,
		seastar::future<>(ServiceAccessorTrait<TService>::ServiceType::*Func)(
			httpd::const_req, httpd::reply&)>
	class ServiceHandler<TService, Func> : public ServiceHandlerBase {
	public:
		using ServiceHandlerBase::ServiceHandlerBase;

		seastar::future<std::unique_ptr<httpd::reply>> handle(
			const seastar::sstring& path,
			std::unique_ptr<httpd::request> request,
			std::unique_ptr<httpd::reply> reply) override {
			return seastar::do_with(
				container_->get<TService>(),
				std::move(request),
				std::move(reply),
				[](auto& instance, auto& request, auto& reply) {
				return (ServiceAccessorTrait<TService>()(instance).*(Func))(*request, *reply)
				.then([&reply] { return std::move(reply); });
			});
		}
	};
}

