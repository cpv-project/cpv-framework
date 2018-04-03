#include <chrono>
#include <core/reactor.hh>
#include <core/shared_future.hh>
#include <net/inet_address.hh>
#include <net/tls.hh>
#include <net/dns.hh>
#include <CPVFramework/Utility/SocketHolder.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Exceptions/NotImplementedException.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Exceptions/NetworkException.hpp>
#include <CPVFramework/Http/Client/HttpClient.hpp>
#include <CPVFramework/Http/Client/HttpClientRequest.hpp>
#include <CPVFramework/Http/Client/HttpClientResponse.hpp>

namespace cpv {
	namespace {
		/** Maximum idle http connections for per host and per core */
		static const std::size_t HttpConnectionPoolSize = 20;
		/** Dns resolve result cache time */
		static const std::chrono::milliseconds DnsCacheTime(30000);
		/** The initial size used to check connection is alive */
		static const std::size_t CheckAliveBytes = 3;
	}

	/** Defines members of HttpClient */
	class HttpClientData :
		public seastar::enable_shared_from_this<HttpClientData> {
	public:
		std::string hostname;
		std::uint16_t port;
		bool useSSL;
		std::string x509Cert; // for ssl client certificate
		std::string x509Key; // for ssl client certificate
		std::string hostHeader; // for build request
		seastar::socket_address ipAddress;
		bool ipAddressIsResolved;
		bool ipAddressIsFixed;
		std::chrono::system_clock::time_point ipAddressUpdatedTime;
		seastar::shared_ptr<seastar::tls::certificate_credentials> certificates;
		seastar::shared_future<> certificatesInitialized;
		std::vector<std::pair<SocketHolder, std::chrono::system_clock::time_point>> connections;
		bool dropIdleConnectionTimerIsRunning;

		/** Close connections not used a while */
		void dropIdleConnectionTimer() {
			if (dropIdleConnectionTimerIsRunning) {
				return;
			}
			dropIdleConnectionTimerIsRunning = true;
		}

		/** Update connection parameters */
		void update(
			const std::string_view& hostnameRef,
			std::uint16_t portVal,
			bool useSSLVal,
			const std::string_view& x509CertRef,
			const std::string_view& x509KeyRef) {
			hostname = hostnameRef;
			port = portVal;
			useSSL = useSSLVal;
			x509Cert = x509KeyRef;
			x509Key = x509KeyRef;
			hostHeader = hostname;
			if (useSSL ? (port != 443) : (port != 80)) {
				hostHeader.append(":");
				dumpIntToDec(port, hostHeader);
			}
			try {
				seastar::net::inet_address inetAddress(hostname);
				if (inetAddress.in_family() == seastar::net::inet_address::family::INET) {
					ipAddress = seastar::socket_address(seastar::ipv4_addr(inetAddress, port));
					ipAddressIsResolved = true;
					ipAddressIsFixed = true;
				} else {
					// seastar's socket_address only support ipv4
					throw NotImplementedException(CPV_CODEINFO, "ipv6 address is unsupported");
				}
			} catch (const std::invalid_argument&) {
				ipAddressIsResolved = false;
				ipAddressIsFixed = false;
				ipAddressUpdatedTime = {};
			}
			if (useSSL) {
				certificates = seastar::make_shared<seastar::tls::certificate_credentials>();
				if (!x509Cert.empty() && !x509Key.empty()) {
					certificates->set_x509_key(x509Cert, x509Key, seastar::tls::x509_crt_format::PEM);
				}
				certificatesInitialized = certificates->set_system_trust();
			}
		}

		/** Create a new plain or ssl connection */
		seastar::future<SocketHolder> newConnection() {
			if (!ipAddressIsResolved) {
				throw LogicException(CPV_CODEINFO, "ip address didn't resolved");
			}
			if (useSSL) {
				if (certificates == nullptr) {
					throw LogicException(CPV_CODEINFO, "ssl certificates is nullptr");
				}
				if (certificatesInitialized.available() && !certificatesInitialized.failed()) {
					// fast path
					return seastar::tls::connect(certificates, ipAddress, "").then([] (auto connected) {
						return SocketHolder(std::move(connected));
					});
				} else {
					// slow path
					auto self = shared_from_this();
					return certificatesInitialized.get_future().then([self] {
						return seastar::tls::connect(self->certificates, self->ipAddress, "");
					}).then([] (auto connected) {
						return SocketHolder(std::move(connected));
					});
				}
			} else {
				return seastar::engine().net().connect(ipAddress).then([] (auto connected) {
					return SocketHolder(std::move(connected));
				});
			}
		}

		/** Get an idle or exists connection */
		seastar::future<SocketHolder> getConnection(bool& reused) {
			if (!connections.empty()) {
				reused = true;
				auto connection = std::move(connections.back());
				connections.pop_back();
				return seastar::make_ready_future<SocketHolder>(std::move(connection.first));
			}
			reused = false;
			if (ipAddressIsResolved && (ipAddressIsFixed ||
				std::chrono::system_clock::now() - ipAddressUpdatedTime <= DnsCacheTime)) {
				return newConnection();
			}
			auto self = shared_from_this();
			return seastar::net::dns::resolve_name(hostname).then([self](auto inetAddress) {
				if (inetAddress.in_family() == seastar::net::inet_address::family::INET) {
					seastar::socket_address address(seastar::ipv4_addr(inetAddress, self->port));
					self->ipAddress = address;
					self->ipAddressIsResolved = true;
					self->ipAddressUpdatedTime = std::chrono::system_clock::now();
					return self->newConnection();
				} else {
					// seastar's socket_address only support ipv4
					return seastar::make_exception_future<SocketHolder>(
						NotImplementedException(CPV_CODEINFO, "ipv6 address is unsupported"));
				}
			});
		}

		/** Return the connection to pool if pool is not full */
		void returnConnection(SocketHolder&& connection) {
			if (!connection.isConnected()) {
				throw LogicException(CPV_CODEINFO, "invalid connection");
			}
			if (connections.size() < HttpConnectionPoolSize) {
				connections.emplace_back(std::move(connection), std::chrono::system_clock::now());
			}
		}

		HttpClientData() :
			hostname(),
			port(),
			useSSL(false),
			x509Cert(),
			x509Key(),
			hostHeader(),
			ipAddress(),
			ipAddressIsResolved(false),
			ipAddressIsFixed(false),
			ipAddressUpdatedTime(),
			certificates(),
			certificatesInitialized(),
			connections(),
			dropIdleConnectionTimerIsRunning(false) { }
	};

	/** Send a http request by it's full conetent */
	seastar::future<HttpClientResponse> HttpClient::send(const std::string_view& str) {
		if (str.size() <= CheckAliveBytes) {
			throw LogicException(CPV_CODEINFO, "content too short");
		}
		return seastar::do_with(
			data_,
			std::string(str),
			false,
			false,
			SocketHolder(),
			HttpClientResponse(), [](
			auto& data,
			auto& str,
			auto& reused,
			auto& detectAlive,
			auto& connection,
			auto& response) {
			return seastar::repeat([&data, &str, &reused, &detectAlive, &connection, &response] {
				return data->getConnection(reused)
				.then([&str, &reused, &detectAlive, &connection] (auto connectionVal) {
					// send request
					connection = std::move(connectionVal);
					if (reused) {
						// send initial bytes to detect the conection is alive without side effect
						// only enable when the connection is reused
						detectAlive = true;
						return connection.out().write(str.data(), CheckAliveBytes).then([&connection] {
							return connection.out().flush();
						}).then([&str, &detectAlive, &connection] {
							detectAlive = false;
							return connection.out().write(
								str.data() + CheckAliveBytes, str.size() - CheckAliveBytes);
						}).then([&connection] {
							return connection.out().flush();
						});
					} else {
						return connection.out().write(str.data(), str.size()).then([&connection] {
							return connection.out().flush();
						});
					}
				}).then([&data, &connection, &response] {
					// receive response
					return seastar::repeat([&connection, &response] {
						return connection.in().read().then([&response] (auto buf) {
							bool eof = response.appendReceived(std::move(buf));
							return eof ? seastar::stop_iteration::yes : seastar::stop_iteration::no;
						});
					}).then([&data, &connection, &response] {
						// return connection to pool if keep alive is enabled
						auto value = response.getHeader("Connection");
						if (value.empty() || caseInsensitiveEquals(value, "Keep-Alive")) {
							data->returnConnection(std::move(connection));
						}
						return seastar::stop_iteration::yes;
					});
				}).handle_exception([&data, &reused, &detectAlive, &response] (std::exception_ptr ex) {
					// retry if reuse connection failed
					if (reused && detectAlive) {
						response = HttpClientResponse();
						return seastar::make_ready_future<seastar::stop_iteration>(
							seastar::stop_iteration::no);
					}
					return seastar::make_exception_future<seastar::stop_iteration>(
						NetworkException(CPV_CODEINFO, "send http request to",
							data->hostHeader, "failed:", ex));
				});
			}).then([&response] {
				return std::move(response);
			});
		});
	}

	/** make a http request */
	HttpClientRequest HttpClient::makeRequest(
		const std::string_view& method,
		const std::string_view& path) {
		return HttpClientRequest(method, path, data_->hostHeader);
	}

	/** Create a plain http client */
	HttpClient HttpClient::create(
		const std::string_view& hostname,
		std::uint16_t port) {
		HttpClient client;
		client.data_->update(hostname, port, false, "", "");
		return client;
	}

	/** Create a ssl encrypted https client */
	HttpClient HttpClient::createSSL(
		const std::string_view& hostname,
		std::uint16_t port,
		const std::string_view& x509Cert,
		const std::string_view& x509Key) {
		HttpClient client;
		client.data_->update(hostname, port, true, x509Cert, x509Key);
		return client;
	}

	/** Constructor */
	HttpClient::HttpClient() :
		data_(seastar::make_shared<HttpClientData>()) { }
}

