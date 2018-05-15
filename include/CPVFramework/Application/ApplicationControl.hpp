#pragma once
#include <core/sleep.hh>
#include <core/distributed.hh>
#include "Application.hpp"

namespace cpv {
	template <class TApplication>
	class ApplicationControl {
	public:
		/** Start applications on every logical core */
		template <class... Args>
		seastar::future<> start(Args&&... args) {
			auto applications = applications_;
			return applications->start(std::forward<Args>(args)...).then([applications] {
				return applications->invoke_on_all([] (TApplication& application) {
					return application.start();
				});
			});
		}

		/** Wait for applications exit by blocking forever until SIGINT is received */
		seastar::future<> wait() const {
			static std::atomic_bool Running;
			Running = true;
			signal(SIGINT, [] (int) { Running = false; });
			return seastar::repeat([] {
				return seastar::sleep(std::chrono::seconds(1)).then([] {
					return Running ?
						seastar::stop_iteration::no :
						seastar::stop_iteration::yes;
				});
			});
		}

		/** Stop started applications */
		seastar::future<> stop() const {
			auto applications = applications_;
			return applications->stop().then([applications] { });
		}

		/** Invoke a method on all applications */
		template <class... Args>
		seastar::future<> invokeOnAll(Args&&... args) const {
			return applications_->invoke_on_all(std::forward<Args>(args)...);
		}

		/** Constructor */
		ApplicationControl() :
			applications_(seastar::make_shared<seastar::distributed<TApplication>>()) { }

	private:
		seastar::shared_ptr<seastar::distributed<TApplication>> applications_;
	};
}

