#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/future.hh>
#include <seastar/core/future-util.hh>
#include <seastar/core/sleep.hh>
#include <seastar/core/thread.hh>
#include <seastar/net/inet_address.hh>
#include <CPVFramework/Utility/SocketHolder.hpp>
#include <CPVFramework/Testing/GTestUtils.hpp>

namespace cpv::gtest {
	/** the main function of test executable */
	int runAllTests(int argc, char** argv) {
		::testing::InitGoogleTest(&argc, argv);
		seastar::app_template app;
		int returnValue(0);
		app.run(argc, argv, [&returnValue] {
			return seastar::async([] {
				return RUN_ALL_TESTS();
			}).then([&returnValue] (int result) {
				returnValue = result;
				// wait for internal cleanup to make leak sanitizer happy
				return seastar::sleep(std::chrono::seconds(1));
			});
		});
		return returnValue;
	}
	
	/** create tcp connection, send request then return received response as string */
	seastar::future<std::string> tcpSendRequest(
		const std::string& ip, std::size_t port, Packet&& p) {
		seastar::socket_address addr(seastar::ipv4_addr(seastar::net::inet_address(ip), port));
		return seastar::engine().net().connect(addr).then([p=std::move(p)] (auto connection) mutable {
			return seastar::do_with(
				cpv::SocketHolder(std::move(connection)), std::move(p), std::string(),
				[] (auto& s, auto& p, auto& str) {
					return (s.out() << std::move(p)).then([&s, &str] {
						return seastar::repeat([&s, &str] {
							return s.in().read().then([&str] (auto buf) {
								if (buf.size() > 0) {
									str.append(buf.get(), buf.size());
									return seastar::stop_iteration::no;
								} else {
									return seastar::stop_iteration::yes;
								}
							});
						});
					}).then([&s] {
						return s.close();
					}).then([&str] {
						return std::move(str);
					});
				});
		});
	}
	
	/** create tcp connection, send request partially then return received response as string */
	seastar::future<std::string> tcpSendPartialRequest(
		const std::string& ip,
		std::size_t port,
		const std::vector<std::string_view>& parts,
		std::chrono::milliseconds interval) {
		seastar::socket_address addr(seastar::ipv4_addr(seastar::net::inet_address(ip), port));
		return seastar::engine().net().connect(addr).then([parts, interval] (auto connection) mutable {
			return seastar::do_with(
				cpv::SocketHolder(std::move(connection)),
				std::string(),
				std::move(parts),
				interval,
				static_cast<std::size_t>(0),
				[] (auto& s, auto& str, auto& parts, auto& interval, auto& index) {
					return seastar::repeat([&s, &parts, &interval, &index] {
						if (index < parts.size()) {
							using namespace cpv;
							Packet p(parts[index++]);
							return (s.out() << std::move(p)).then([&interval] {
								return seastar::sleep(interval);
							}).then([] {
								return seastar::stop_iteration::no;
							}).handle_exception([] (std::exception_ptr) {
								// ignore error for limitation testing
								return seastar::stop_iteration::yes;
							});
						} else {
							return seastar::make_ready_future<seastar::stop_iteration>(
								seastar::stop_iteration::yes);
						}
					}).then([&s, &str] {
						return seastar::repeat([&s, &str] {
							return s.in().read().then([&str] (auto buf) {
								if (buf.size() > 0) {
									str.append(buf.get(), buf.size());
									return seastar::stop_iteration::no;
								} else {
									return seastar::stop_iteration::yes;
								}
							});
						});
					}).then([&s] {
						return s.close();
					}).then([&str] {
						return std::move(str);
					});
				});
		});
	}
}

