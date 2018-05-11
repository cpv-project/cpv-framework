#include <core/reactor.hh>
#include <core/sleep.hh>
#include <net/dns.hh>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Exceptions/NotImplementedException.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Exceptions/NetworkException.hpp>
#include "HttpClientImpl.hpp"
#include "HttpClientRequestImpl.hpp"
#include "HttpClientResponseImpl.hpp"

namespace cpv {
	/** Return whether it's a https client */
	bool HttpClientImpl::isSSL() const {
		return useSSL_;
	}

	/** make a http request */
	Object<HttpClientRequest> HttpClientImpl::makeRequest(
		const std::string_view& method, const std::string_view& path) {
		return makeObject<HttpClientRequestImpl>(method, path, hostHeader_).cast<HttpClientRequest>();
	}

	/** Send a http request */
	seastar::future<Object<HttpClientResponse>> HttpClientImpl::send(Object<HttpClientRequest>&& request) {
		auto self = shared_from_this();
		return seastar::do_with(
			std::move(self),
			std::move(request),
			false,
			false,
			SocketHolder(),
			makeObject<HttpClientResponseImpl>(), [](
			auto& self,
			auto& request,
			auto& reused,
			auto& detectAlive,
			auto& connection,
			auto& response) {
			return seastar::repeat([&self, &request, &reused, &detectAlive, &connection, &response] {
				return self->getConnection(reused)
				.then([&self, &request, &reused, &detectAlive, &connection] (auto connectionVal) {
					// send request
					connection = std::move(connectionVal);
					auto str = request->str();
					auto checkAliveBytes = self->checkAliveBytes_;
					if (str.size() <= checkAliveBytes) {
						return seastar::make_exception_future<>(
							LogicException(CPV_CODEINFO, "content too short"));
					}
					if (reused) {
						// send initial bytes to detect the conection is alive without side effect
						// only enable when the connection is reused
						detectAlive = true;
						return connection.out().write(str.data(), checkAliveBytes).then([&connection] {
							return connection.out().flush();
						}).then([str, checkAliveBytes, &detectAlive, &connection] {
							detectAlive = false;
							return connection.out().write(
								str.data() + checkAliveBytes, str.size() - checkAliveBytes);
						}).then([&connection] {
							return connection.out().flush();
						});
					} else {
						return connection.out().write(str.data(), str.size()).then([&connection] {
							return connection.out().flush();
						});
					}
				}).then([&self, &connection, &response] {
					// receive response
					return seastar::repeat([&connection, &response] {
						return connection.in().read().then([&response] (auto buf) {
							bool eof = response->appendReceived({ buf.get(), buf.size() });
							return eof ? seastar::stop_iteration::yes : seastar::stop_iteration::no;
						});
					}).then([&self, &connection, &response] {
						// return connection to pool if keep alive is enabled
						auto value = response->getHeader("Connection");
						if (value.empty() || caseInsensitiveEquals(value, "Keep-Alive")) {
							self->returnConnection(std::move(connection));
						}
						return seastar::stop_iteration::yes;
					});
				}).handle_exception([&self, &reused, &detectAlive, &response] (std::exception_ptr ex) {
					// retry if reuse connection failed
					if (reused && detectAlive) {
						response = makeObject<HttpClientResponseImpl>();
						return seastar::make_ready_future<seastar::stop_iteration>(
							seastar::stop_iteration::no);
					}
					return seastar::make_exception_future<seastar::stop_iteration>(
						NetworkException(CPV_CODEINFO, "send http request to",
							self->hostHeader_, "failed:", ex));
				});
			}).then([&response] {
				return std::move(response).template cast<HttpClientResponse>();
			});
		});
	}

	/** Constructor */
	HttpClientImpl::HttpClientImpl(
		const std::string_view& hostname,
		std::uint16_t port,
		bool useSSL,
		const std::string_view& x509Cert,
		const std::string_view& x509Key,
		std::size_t poolSize,
		const std::chrono::milliseconds& dnsCacheTime,
		const std::chrono::milliseconds& keepAliveTime,
		const std::chrono::milliseconds& checkDropInterval,
		std::size_t checkAliveBytes) :
		hostname_(hostname),
		port_(port),
		useSSL_(useSSL),
		x509Cert_(x509Cert),
		x509Key_(x509Key),
		poolSize_(poolSize),
		dnsCacheTime_(dnsCacheTime),
		keepAliveTime_(keepAliveTime),
		checkDropInterval_(checkDropInterval),
		checkAliveBytes_(checkAliveBytes),
		hostHeader_(hostname),
		ipAddress_(),
		ipAddressIsResolved_(false),
		ipAddressIsFixed_(false),
		ipAddressUpdatedTime_(),
		certificates_(),
		certificatesInitialized_(),
		connections_(),
		dropIdleConnectionTimerIsRunning_(false) {
		// build header content of HOST
		if (useSSL_ ? (port_ != 443) : (port_ != 80)) {
			hostHeader_.append(":");
			dumpIntToDec(port_, hostHeader_);
		}
		// try to parse hostname as ip address
		try {
			seastar::net::inet_address inetAddress(hostname_);
			if (inetAddress.in_family() == seastar::net::inet_address::family::INET) {
				ipAddress_ = seastar::socket_address(seastar::ipv4_addr(inetAddress, port));
				ipAddressIsResolved_ = true;
				ipAddressIsFixed_ = true;
			} else {
				// seastar's socket_address only support ipv4
				throw NotImplementedException(CPV_CODEINFO, "ipv6 address is unsupported");
			}
		} catch (const std::invalid_argument&) {
			ipAddressIsResolved_ = false;
			ipAddressIsFixed_ = false;
			ipAddressUpdatedTime_ = {};
		}
		// setup ssl certificates
		if (useSSL_) {
			certificates_ = seastar::make_shared<seastar::tls::certificate_credentials>();
			if (!x509Cert_.empty() && !x509Key_.empty()) {
				certificates_->set_x509_key(x509Cert_, x509Key_, seastar::tls::x509_crt_format::PEM);
			}
			certificatesInitialized_ = certificates_->set_system_trust();
		}
	}

	/** Close connections not used a while */
	void HttpClientImpl::dropIdleConnectionTimer() {
		if (dropIdleConnectionTimerIsRunning_) {
			return;
		}
		dropIdleConnectionTimerIsRunning_ = true;
		seastar::do_with(
			weak_from_this(),
			checkDropInterval_,
			[] (auto& self, auto& checkDropInterval) {
			return seastar::repeat([&self, &checkDropInterval] {
				return seastar::sleep(checkDropInterval).then([&self] {
					if (self.get() == nullptr) {
						return seastar::stop_iteration::yes;
					}
					auto now = std::chrono::system_clock::now();
					self->connections_.erase(std::remove_if(
						self->connections_.begin(),
						self->connections_.end(),
						[&self, &now] (const auto& connection) {
							return now - connection.second > self->keepAliveTime_;
						}),
						self->connections_.end());
					return self->connections_.empty() ?
						seastar::stop_iteration::yes :
						seastar::stop_iteration::no;
				});
			}).finally([&self] {
				if (self.get() != nullptr) {
					self->dropIdleConnectionTimerIsRunning_ = false;
				}
			});
		});
	}

	/** Create a new plain or ssl connection */
	seastar::future<SocketHolder> HttpClientImpl::newConnection() {
		if (!ipAddressIsResolved_) {
			throw LogicException(CPV_CODEINFO, "ip address didn't resolved");
		}
		if (useSSL_) {
			if (certificates_ == nullptr) {
				throw LogicException(CPV_CODEINFO, "ssl certificates is nullptr");
			}
			if (certificatesInitialized_.available() && !certificatesInitialized_.failed()) {
				// fast path
				return seastar::tls::connect(certificates_, ipAddress_, "").then([] (auto connected) {
					return SocketHolder(std::move(connected));
				});
			} else {
				// slow path
				auto self = shared_from_this();
				return certificatesInitialized_.get_future().then([self] {
					return seastar::tls::connect(self->certificates_, self->ipAddress_, "");
				}).then([] (auto connected) {
					return SocketHolder(std::move(connected));
				});
			}
		} else {
			return seastar::engine().net().connect(ipAddress_).then([] (auto connected) {
				return SocketHolder(std::move(connected));
			});
		}
	}

	/** Get a new or reused connection */
	seastar::future<SocketHolder> HttpClientImpl::getConnection(bool& reused) {
		if (!connections_.empty()) {
			reused = true;
			auto connection = std::move(connections_.back());
			connections_.pop_back();
			return seastar::make_ready_future<SocketHolder>(std::move(connection.first));
		}
		reused = false;
		if (ipAddressIsResolved_ && (ipAddressIsFixed_ ||
			std::chrono::system_clock::now() - ipAddressUpdatedTime_ <= dnsCacheTime_)) {
			return newConnection();
		}
		auto self = shared_from_this();
		return seastar::net::dns::resolve_name(hostname_).then([self](auto inetAddress) {
			if (inetAddress.in_family() == seastar::net::inet_address::family::INET) {
				seastar::socket_address address(seastar::ipv4_addr(inetAddress, self->port_));
				self->ipAddress_ = address;
				self->ipAddressIsResolved_ = true;
				self->ipAddressUpdatedTime_ = std::chrono::system_clock::now();
				return self->newConnection();
			} else {
				// seastar's socket_address only support ipv4
				return seastar::make_exception_future<SocketHolder>(
					NotImplementedException(CPV_CODEINFO, "ipv6 address is unsupported"));
			}
		});
	}

	/** Return the connection to pool if pool is not full */
	void HttpClientImpl::returnConnection(SocketHolder&& connection) {
		if (!connection.isConnected()) {
			throw LogicException(CPV_CODEINFO, "invalid connection");
		}
		if (connections_.size() < poolSize_) {
			connections_.emplace_back(std::move(connection), std::chrono::system_clock::now());
			dropIdleConnectionTimer();
		}
	}
}

