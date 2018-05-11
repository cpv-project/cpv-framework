#pragma once
#include <chrono>
#include <core/reactor.hh>
#include <core/shared_ptr.hh>
#include <core/weak_ptr.hh>
#include <core/shared_future.hh>
#include <net/inet_address.hh>
#include <net/tls.hh>
#include <CPVFramework/Http/Client/HttpClient.hpp>
#include <CPVFramework/Http/Client/HttpClientRequest.hpp>
#include <CPVFramework/Http/Client/HttpClientResponse.hpp>
#include <CPVFramework/Utility/SocketHolder.hpp>

namespace cpv {
	/** Implementation of http client */
	class HttpClientImpl :
		public HttpClient,
		public seastar::enable_shared_from_this<HttpClientImpl>,
		public seastar::weakly_referencable<HttpClientImpl> {
	public:
		/** Return whether it's a https client */
		bool isSSL() const override;

		/** make a http request */
		Object<HttpClientRequest> makeRequest(
			const std::string_view& method, const std::string_view& path) override;

		/** Send a http request */
		seastar::future<Object<HttpClientResponse>> send(Object<HttpClientRequest>&& request) override;

		/** Constructor */
		HttpClientImpl(
			const std::string_view& hostname,
			std::uint16_t port,
			bool useSSL,
			const std::string_view& x509Cert,
			const std::string_view& x509Key,
			std::size_t poolSize = 20,
			const std::chrono::milliseconds& dnsCacheTime = std::chrono::milliseconds(30000),
			const std::chrono::milliseconds& keepAliveTime = std::chrono::milliseconds(3000),
			const std::chrono::milliseconds& checkDropInterval = std::chrono::milliseconds(1000),
			std::size_t checkAliveBytes = 3);

	private:
		/** Close connections not used a while */
		void dropIdleConnectionTimer();

		/** Create a new plain or ssl connection */
		seastar::future<SocketHolder> newConnection();

		/** Get a new or reused connection */
		seastar::future<SocketHolder> getConnection(bool& reused);

		/** Return the connection to pool if pool is not full */
		void returnConnection(SocketHolder&& connection);

	private:
		std::string hostname_;
		std::uint16_t port_;
		bool useSSL_;
		/** For ssl client certificate */
		std::string x509Cert_;
		/** For ssl client certificate */
		std::string x509Key_;
		/** Maximum idle http connections for per host and per core */
		std::size_t poolSize_;
		/** Dns resolve result cache time */
		std::chrono::milliseconds dnsCacheTime_;
		/** How long a connection can keep idle, the default timeout of apache 2.4 is 5s */
		std::chrono::milliseconds keepAliveTime_;
		/** The interval of drop idle connection timer */
		std::chrono::milliseconds checkDropInterval_;
		/** The initial size used to check connection is alive */
		std::size_t checkAliveBytes_;
		/** For request building */
		std::string hostHeader_;
		seastar::socket_address ipAddress_;
		bool ipAddressIsResolved_;
		bool ipAddressIsFixed_;
		std::chrono::system_clock::time_point ipAddressUpdatedTime_;
		seastar::shared_ptr<seastar::tls::certificate_credentials> certificates_;
		seastar::shared_future<> certificatesInitialized_;
		std::vector<std::pair<SocketHolder, std::chrono::system_clock::time_point>> connections_;
		bool dropIdleConnectionTimerIsRunning_;
	};
}

