#include <utility>
#include <seastar/core/timer.hh>
#include <seastar/core/shared_future.hh>
#include <seastar/net/api.hh>
#include <seastar/core/reactor.hh>
#include <CPVFramework/HttpServer/HttpServer.hpp>
#include <CPVFramework/Utility/NetworkUtils.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Exceptions/NotImplementedException.hpp>
#include "./Connections/Http11ServerConnection.hpp"
#include "./HttpServerSharedData.hpp"

// increase backlog can avoid ECONNREFUSED when many connections come with in a short period
#if !defined(CPV_HTTP_SERVER_LISTEN_BACKLOG)
	#define CPV_HTTP_SERVER_LISTEN_BACKLOG 65535
#endif

namespace cpv {
	/** Members of HttpServer */
	class HttpServerData {
	public:
		/** Constructor */
		HttpServerData(const Container& container) :
			connectionsWrapper(seastar::make_lw_shared<HttpServerConnectionsWrapper>()),
			sharedData(seastar::make_lw_shared<HttpServerSharedData>(
				container, connectionsWrapper->weak_from_this())),
			listeners(),
			listenerStoppedFutures(),
			detectTimeoutTimer(),
			stopping(false) { }
		
	public:
		seastar::lw_shared_ptr<HttpServerConnectionsWrapper> connectionsWrapper;
		seastar::lw_shared_ptr<HttpServerSharedData> sharedData;
		std::vector<seastar::lw_shared_ptr<
			std::pair<seastar::server_socket, seastar::socket_address>>> listeners;
		std::vector<seastar::future<>> listenerStoppedFutures;
		seastar::timer<> detectTimeoutTimer;
		bool stopping;
	};
	
	/** Start accept http connections */
	seastar::future<> HttpServer::start() {
		// check state
		if (CPV_UNLIKELY(data_->stopping)) {
			return seastar::make_exception_future(LogicException(
				CPV_CODEINFO, "can't start http server while stopping"));
		}
		data_->sharedData->logger->log(LogLevel::Notice, "starting http server");
		// cleanup (if last time this function interrupt by exception)
		for (auto& listener : data_->listeners) {
			listener->first.abort_accept();
		}
		data_->listeners.clear();
		data_->listenerStoppedFutures.clear();
		// parse listen addresses and create listeners
		seastar::listen_options listenOptions;
		listenOptions.reuse_address = true;
		// cppcheck-suppress ConfigurationNotChecked
		listenOptions.listen_backlog = CPV_HTTP_SERVER_LISTEN_BACKLOG;
		for (const auto& listenAddressStr : data_->sharedData->configuration.getListenAddresses()) {
			auto listenAddress = parseListenAddress(listenAddressStr);
			auto listener = seastar::listen(listenAddress, listenOptions);
			data_->listeners.emplace_back(seastar::make_lw_shared<
				std::pair<seastar::server_socket, seastar::socket_address>>(
				std::move(listener), std::move(listenAddress)));
		}
		// start timer
		data_->detectTimeoutTimer.arm(
			seastar::timer<>::clock::now(),
			data_->sharedData->configuration.getRequestTimeout());
		// start listeners
		for (const auto& listener : data_->listeners) {
			data_->sharedData->logger->log(LogLevel::Notice,
				"start listen http connections from:", listener->second);
			data_->listenerStoppedFutures.emplace_back(seastar::keep_doing(
				[listener, connectionsWrapper=data_->connectionsWrapper, sharedData=data_->sharedData] {
				return listener->first.accept().then(
					[connectionsWrapper, sharedData] (seastar::accept_result ar) {
					// currently only support http 1.0/1.1
					sharedData->logger->log(LogLevel::Info,
						"accepted http connection from:", ar.remote_address,
						", connections count:", connectionsWrapper->value.size() + 1);
					auto connection = seastar::make_shared<Http11ServerConnection>(
						sharedData, std::move(ar.connection), std::move(ar.remote_address));
					connection->start();
					connectionsWrapper->value.emplace(std::move(connection));
					sharedData->metricData.total_connections += 1;
					sharedData->metricData.current_connections = connectionsWrapper->value.size();
				});
			}).handle_exception([listener, sharedData=data_->sharedData] (std::exception_ptr ex) {
				sharedData->logger->log(LogLevel::Notice,
					"stop listen http connections from:", listener->second, "because of", ex);
			}));
		}
		data_->sharedData->logger->log(LogLevel::Notice, "http server started");
		return seastar::make_ready_future<>();
	}
	
	/** Stop accept http connection and close all exists connections  */
	seastar::future<> HttpServer::stop() {
		data_->sharedData->logger->log(LogLevel::Notice, "stopping http server");
		// stop timer
		data_->detectTimeoutTimer.cancel();
		// abort all listeners
		for (const auto& listener : data_->listeners) {
			listener->first.abort_accept();
		}
		return seastar::when_all(
			data_->listenerStoppedFutures.begin(),
			data_->listenerStoppedFutures.end()).then([this] (auto&&) {
			// clean all listeners
			data_->listeners.clear();
			data_->listenerStoppedFutures.clear();
			// close all connections
			auto futures = std::vector<seastar::future<>>();
			auto connectionsCopy = data_->connectionsWrapper->value;
			for (const auto& connection : connectionsCopy) {
				// connections may remove themself from collection while iterating
				// so iterate a copied collection
				futures.emplace_back(connection->stop());
			}
			return seastar::when_all(futures.begin(), futures.end()).then([this] (auto&&) {
				// clean all connections
				data_->connectionsWrapper->value.clear();
				data_->sharedData->logger->log(LogLevel::Notice, "http server stopped");
				data_->sharedData->metricData.current_connections = 0;
			});
		});
	}
	
	/** Constructor */
	HttpServer::HttpServer(const Container& container) :
		data_(std::make_unique<HttpServerData>(container)) {
		// detect timeout for all connections.
		// it's better than let connections manage their own timer,
		// because add timer and remove timer is much heavy than just iterate connections.
		data_->detectTimeoutTimer.set_callback([connectionsWrapper=data_->connectionsWrapper] {
			std::vector<seastar::shared_ptr<HttpServerConnectionBase>> timeoutReached;
			for (auto& connection : connectionsWrapper->value) {
				if (connection->detectTimeoutFlag_) {
					// connection didn't set this value to false since previous tick
					timeoutReached.emplace_back(connection);
				} else {
					connection->detectTimeoutFlag_ = true;
				}
			}
			for (auto& connection : timeoutReached) {
				connection->onTimeout();
			}
		});
	}
	
	/** Move constructor (for incomplete member type) */
	HttpServer::HttpServer(HttpServer&&) = default;
	
	/** Move assign operator (for incomplete member type) */
	HttpServer& HttpServer::operator=(HttpServer&&) = default;

	/** Destructor (for incomplete type HttpServerData) */
	HttpServer::~HttpServer() = default;
}

