#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include "./HttpServerConfigurationData.hpp"

namespace cpv {
	/** Get listen addresses, e.g. "0.0.0.0:80" */
	const std::vector<std::string>& HttpServerConfiguration::getListenAddresses() const& {
		return data_->listenAddresses;
	}
	
	/** Set listen address */
	void HttpServerConfiguration::setListenAddresses(std::vector<std::string>&& listenAddresses) {
		data_->listenAddresses = listenAddresses;
	}
	
	/** Get bytes limitation of initial request data */
	std::size_t HttpServerConfiguration::getMaxInitialRequestBytes() const {
		return data_->maxInitialRequestBytes;
	}
	
	/** Set bytes limitation of initial request data */
	void HttpServerConfiguration::setMaxInitialRequestBytes(std::size_t maxInitialRequestBytes) {
		data_->maxInitialRequestBytes = maxInitialRequestBytes;
	}
	
	/** Get packets limitation of initial request data */
	std::size_t HttpServerConfiguration::getMaxInitialRequestPackets() const {
		return data_->maxInitialRequestPackets;
	}
	
	/** Set packets limitation of initial request data */
	void HttpServerConfiguration::setMaxInitialRequestPackets(std::size_t maxInitialRequestPackets) {
		data_->maxInitialRequestPackets = maxInitialRequestPackets;
	}
	
	/** Constructor */
	HttpServerConfiguration::HttpServerConfiguration() :
		data_(std::make_unique<HttpServerConfigurationData>()) { }
	
	/** Destructor */
	HttpServerConfiguration::~HttpServerConfiguration() = default;
	
	/** Copy constructor */
	HttpServerConfiguration::HttpServerConfiguration(const HttpServerConfiguration& other) :
		data_(std::make_unique<HttpServerConfigurationData>(*other.data_)) { }
	
	/** Move constructor */
	HttpServerConfiguration::HttpServerConfiguration(HttpServerConfiguration&& other) :
		HttpServerConfiguration(other) {
		// same as copy, disallow empty pointer
	}
	
	/** Copy assignment */
	HttpServerConfiguration& HttpServerConfiguration::operator=(const HttpServerConfiguration& other) {
		if (this != &other) {
			data_ = std::make_unique<HttpServerConfigurationData>(*other.data_);
		}
		return *this;
	}
	
	/** Move assignment */
	HttpServerConfiguration& HttpServerConfiguration::operator=(HttpServerConfiguration&& other) {
		// same as copy, disallow empty pointer
		return *this = other;
	}
}

