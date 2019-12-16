#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>

namespace cpv {
	static const std::size_t DefaultMaxInitialRequestBytes = 524288;
	static const std::size_t DefaultMaxInitialRequestPackets = 512;
	static const std::size_t DefaultRequestTimeout = 60000;
	static const std::size_t DefaultRequestQueueSize = 100;
	static const std::size_t DefaultRequestBodySize = 50;
	
	/** Members of HttpServerConfiguration */
	class HttpServerConfigurationData {
	public:
		std::vector<std::string> listenAddresses;
		std::size_t maxInitialRequestBytes;
		std::size_t maxInitialRequestPackets;
		std::chrono::milliseconds requestTimeout;
		std::size_t requestQueueSize;
		std::size_t requestBodyQueueSize;
		
		/** Constructor */
		HttpServerConfigurationData() :
			listenAddresses(),
			maxInitialRequestBytes(DefaultMaxInitialRequestBytes),
			maxInitialRequestPackets(DefaultMaxInitialRequestPackets),
			requestTimeout(DefaultRequestTimeout),
			requestQueueSize(DefaultRequestQueueSize),
			requestBodyQueueSize(DefaultRequestBodySize) { }
	};
	
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
	
	/** Get timeout of initial request in milliseconds */
	std::chrono::milliseconds HttpServerConfiguration::getRequestTimeout() const {
		return data_->requestTimeout;
	}
	
	/** Set timeout of initial request in milliseconds */
	void HttpServerConfiguration::setRequestTimeout(
		const std::chrono::milliseconds& requestTimeout) {
		data_->requestTimeout = requestTimeout;
	}
	
	/** Get the queue size of pending requests of single connection */
	std::size_t HttpServerConfiguration::getRequestQueueSize() const {
		return data_->requestQueueSize;
	}
	
	/** Set the queue size of pending requests of single connection */
	void HttpServerConfiguration::setRequestQueueSize(std::size_t requestQueueSize) {
		data_->requestQueueSize = requestQueueSize;
	}
	
	/** Get the queue size of pending body buffers of single connection */
	std::size_t HttpServerConfiguration::getRequestBodyQueueSize() const {
		return data_->requestBodyQueueSize;
	}
	
	/** Set the queue size of pending body buffers of single connnection */
	void HttpServerConfiguration::setRequestBodyQueueSize(std::size_t requestBodyQueueSize) {
		data_->requestBodyQueueSize = requestBodyQueueSize;
	}
	
	/** Parse from json */
	bool HttpServerConfiguration::loadJson(const cpv::JsonValue& value) {
		data_->listenAddresses << value["listenAddresses"];
		data_->maxInitialRequestBytes << value["maxInitialRequestBytes"];
		data_->maxInitialRequestPackets << value["maxInitialRequestPackets"];
		data_->requestTimeout << value["requestTimeout"];
		data_->requestQueueSize << value["requestQueueSize"];
		data_->requestBodyQueueSize << value["requestBodyQueueSize"];
		return true;
	}
	
	/** Dump to json */
	void HttpServerConfiguration::dumpJson(cpv::JsonBuilder& builder) const {
		builder.startObject()
			.addMember(CPV_JSONKEY("listenAddresses"), data_->listenAddresses)
			.addMember(CPV_JSONKEY("maxInitialRequestBytes"), data_->maxInitialRequestBytes)
			.addMember(CPV_JSONKEY("maxInitialRequestPackets"), data_->maxInitialRequestPackets)
			.addMember(CPV_JSONKEY("requestTimeout"), data_->requestTimeout)
			.addMember(CPV_JSONKEY("requestQueueSize"), data_->requestQueueSize)
			.addMember(CPV_JSONKEY("requestBodyQueueSize"), data_->requestBodyQueueSize)
			.endObject();
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

