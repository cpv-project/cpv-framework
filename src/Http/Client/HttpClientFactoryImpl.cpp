#include <CPVFramework/Http/Client/HttpClientFactoryImpl.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include "HttpClientImpl.hpp"

namespace cpv {
	/** Create a plain http client */
	seastar::shared_ptr<HttpClient> HttpClientFactoryImpl::create(
		const std::string_view& hostname,
		std::uint16_t port) {
		auto key = joinString(":", hostname, port);
		auto it = clients_.find(key);
		if (it == clients_.end()) {
			auto client = seastar::make_shared<HttpClientImpl>(
				hostname, port, false, "", "");
			clients_.emplace(key, client);
			return client;
		} else if (it->second->isSSL()) {
			throw LogicException(CPV_CODEINFO, "http client to", key, "already created and use https");
		} else {
			return it->second;
		}
	}

	/** Create a ssl encrypted https client */
	seastar::shared_ptr<HttpClient> HttpClientFactoryImpl::createSSL(
		const std::string_view& hostname,
		std::uint16_t port,
		const std::string_view& x509Cert,
		const std::string_view& x509Key) {
		auto key = joinString(":", hostname, port);
		auto it = clients_.find(key);
		if (it == clients_.end()) {
			auto client = seastar::make_shared<HttpClientImpl>(
				hostname, port, true, x509Cert, x509Key);
			clients_.emplace(key, client);
			return client;
		} else if (!it->second->isSSL()) {
			throw LogicException(CPV_CODEINFO, "http client to", key, "already created and use plain http");
		} else {
			return it->second;
		}
	}

	/** Constructor */
	HttpClientFactoryImpl::HttpClientFactoryImpl() :
		clients_() { }
}

