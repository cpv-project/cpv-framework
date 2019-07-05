#include <seastar/core/sleep.hh>
#include <seastar/net/packet.hh>
#include <CPVFramework/Exceptions/FormatException.hpp>
#include <CPVFramework/Exceptions/LengthException.hpp>
#include <CPVFramework/Exceptions/LogicException.hpp>
#include <CPVFramework/Utility/BufferUtils.hpp>
#include <CPVFramework/Utility/ConstantStrings.hpp>
#include <CPVFramework/Utility/DateUtils.hpp>
#include <CPVFramework/Utility/EnumUtils.hpp>
#include <CPVFramework/Utility/PacketUtils.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include "./Http11ServerConnection.hpp"
#include "./Http11ServerConnectionRequestStream.hpp"
#include "./Http11ServerConnectionResponseStream.hpp"

namespace cpv {
	namespace {
		/** Static response strings */
		namespace {
			static const std::string ReachedBytesLimitationOfInitialRequestData(
				"HTTP/1.0 400 Bad Request\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n"
				"Content-Length: 58\r\n"
				"Connection: close\r\n\r\n"
				"Error: reached bytes limitation of initial request data.\r\n");
			
			static const std::string ReachedPacketsLimitationOfInitialRequestData(
				"HTTP/1.0 400 Bad Request\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n"
				"Content-Length: 60\r\n"
				"Connection: close\r\n\r\n"
				"Error: reached packets limitation of initial request data.\r\n");
			
			static const std::string InvalidHttpRequestFormat(
				"HTTP/1.0 400 Bad Request\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n"
				"Content-Length: 37\r\n"
				"Connection: close\r\n\r\n"
				"Error: invalid http request format.\r\n");
			
			static const std::string InvalidStateAfterReceivedSingleRequest(
				"HTTP/1.0 500 Internal Server Error\r\n"
				"Content-Type: text/plain;charset=utf-8\r\n"
				"Content-Length: 53\r\n"
				"Connection: close\r\n\r\n"
				"Error: invalid state after received single request.\r\n");
		}
		
		/**
		 * Override global setting of max header size in http_parser.
		 * The header size is checked in this framework, the parser library doesn't have to check it.
		 */
		static const bool HttpMaxHeaderSizeIsOverridden = ([] {
			#if HTTP_PARSER_VERSION_MAJOR > 2 || \
				(HTTP_PARSER_VERSION_MAJOR == 2 && HTTP_PARSER_VERSION_MINOR >= 9)
				internal::http_parser::http_parser_set_max_header_size(
					std::numeric_limits<std::uint32_t>::max());
				return true;
			#else
				return false;
			#endif
		})();
		
		/** Reply static response string and flush the output stream */
		static seastar::future<> replyStaticResponse(SocketHolder& s, const std::string_view& str) {
			return s.out().put(seastar::net::packet::from_static_data(str.data(), str.size()))
				.then([&s] { s.out().flush(); });
		}
		
		/** Move temporary buffer to request and update string view if necessary */
		static inline void flushMergedTemporaryBuffer(
			HttpRequest& request,
			seastar::temporary_buffer<char>& buffer) {
			if (CPV_UNLIKELY(buffer.size() > 0)) {
				request.addUnderlyingBuffer(std::move(buffer));
			}
		}
		
		/** Get http version string from parser, return empty for version not supported */
		static inline std::string_view getHttpVersionString(
			const internal::http_parser::http_parser& parser) {
			if (CPV_LIKELY(parser.http_major == 1)) {
				if (CPV_LIKELY(parser.http_minor == 1)) {
					return constants::Http11;
				} else if (parser.http_minor == 0) {
					return constants::Http10;
				} else if (parser.http_minor == 2) {
					return constants::Http12;
				}
			}
			return std::string_view();
		}
		
		/**
		 * Determine whether should keep connection for next request,
		 * by check http version and connection header.
		 */
		static inline bool shouldKeepConnectionForConnectionHeader(
			const HttpRequest& request,
			const std::string_view& responseVersion) {
			// check http version and connection header from request
			auto& requestHeaders = request.getHeaders();
			auto requestConnectionIt = requestHeaders.find(constants::Connection);
			if (CPV_LIKELY(requestConnectionIt != requestHeaders.end())) {
				if (CPV_LIKELY(requestConnectionIt->second == constants::Keepalive)) {
					// client wants to keepalive
					// many browser will send connection header even for http 1.1
					return true;
				} else {
					// client doesn't want to keepalive
					// it may be "close" or other unsupported string literal
					return false;
				}
			} else if (CPV_UNLIKELY(responseVersion == constants::Http10)) {
				// for http 1.0 connection is not keepalive by default
				return false;
			} else {
				// for http > 1.0 connection is keepalive by default
				return true;
			}
		}
		
		/**
		 * Determine whether should keep connection for next request,
		 * by check content length and received/sent state of request and response.
		 */
		static inline bool shouldKeepConnectionForContentLength(
			const HttpResponse& response,
			std::size_t responseBodyWrittenSize,
			bool requestCompleted,
			const seastar::shared_ptr<Logger>& logger,
			const seastar::socket_address& clientAddress) {
			// check whether content length is fixed or chunked
			auto& responseHeaders = response.getHeaders();
			auto contentLengthIt = responseHeaders.find(constants::ContentLength);
			if (CPV_UNLIKELY(contentLengthIt == responseHeaders.end())) {
				// content length is not fixed, check transfer encoding
				auto transferEncodingIt = responseHeaders.find(constants::TransferEncoding);
				if (transferEncodingIt == responseHeaders.end() ||
					transferEncodingIt->second != constants::Chunked) {
					// close connection to indicate response is end
					return false;
				}
			} else {
				// check whether content length of response is matched to written size
				std::size_t contentLength = 0;
				if (CPV_UNLIKELY(!loadIntFromDec(
					contentLengthIt->second.data(),
					contentLengthIt->second.size(), contentLength))) {
					logger->log(LogLevel::Warning,
						"going to close inconsistent connection from", clientAddress,
						"because content length of response isn't integer");
					return false;
				}
				if (CPV_UNLIKELY(contentLength != responseBodyWrittenSize)) {
					logger->log(LogLevel::Warning,
						"going to close inconsistent connection from", clientAddress,
						"because content length of response isn't matched to written size");
					return false;
				}
			}
			// check whether request content is completely received
			if (CPV_UNLIKELY(!requestCompleted)) {
				// close connection to discard remain parts
				return false;
			}
			// request content is completely received,
			// and response content is written with fixed size or chunked encoding
			return true;
		}
	}
	
	/** Enum descriptions of Http11ServerConnectionState */
	const std::vector<std::pair<Http11ServerConnectionState, const char*>>&
		EnumDescriptions<Http11ServerConnectionState>::get() {
		static std::vector<std::pair<Http11ServerConnectionState, const char*>> staticNames({
			{ Http11ServerConnectionState::Initial, "Initial" },
			{ Http11ServerConnectionState::Started, "Started" },
			{ Http11ServerConnectionState::ReceiveRequestInitial, "ReceiveRequestInitial" },
			{ Http11ServerConnectionState::ReceiveRequestMessageBegin, "ReceiveRequestMessageBegin" },
			{ Http11ServerConnectionState::ReceiveRequestUrl, "ReceiveRequestUrl" },
			{ Http11ServerConnectionState::ReceiveRequestHeaderField, "ReceiveRequestHeaderField" },
			{ Http11ServerConnectionState::ReceiveRequestHeaderValue, "ReceiveRequestHeaderValue" },
			{ Http11ServerConnectionState::ReceiveRequestHeadersComplete, "ReceiveRequestHeadersComplete" },
			{ Http11ServerConnectionState::ReceiveRequestBody, "ReceiveRequestBody" },
			{ Http11ServerConnectionState::ReceiveRequestMessageComplete, "ReceiveRequestMessageComplete" },
			{ Http11ServerConnectionState::ReplyResponse, "ReplyResponse" },
			{ Http11ServerConnectionState::Closing, "Closing" },
			{ Http11ServerConnectionState::Closed, "Closed" },
		});
		return staticNames;
	}
	
	/** Start receive requests and send responses */
	void Http11ServerConnection::start() {
		// check state
		if (CPV_UNLIKELY(state_ != Http11ServerConnectionState::Initial)) {
			throw LogicException(
				CPV_CODEINFO, "can't start http connection not at initial state");
		}
		state_ = Http11ServerConnectionState::Started;
		// handle requests
		auto self = shared_from_this();
		seastar::do_until(
			[self] { return self->state_ == Http11ServerConnectionState::Closing; },
			[self] {
			// handle single request
			self->state_ = Http11ServerConnectionState::ReceiveRequestInitial;
			// initialize http parser
			internal::http_parser::http_parser_init(
				&self->parser_, internal::http_parser::HTTP_REQUEST);
			// reset members if it's not the first request
			if (self->temporaryData_.messageCompleted) {
				self->request_ = {};
				self->response_ = {};
				self->temporaryData_ = {};
			}
			return self->receiveSingleRequest().then([self] {
				self->sharedData_->metricData.request_received += 1;
				return self->replySingleResponse();
			});
		}).handle_exception([self] (std::exception_ptr ex) {
			self->sharedData_->metricData.request_errors += 1;
			self->sharedData_->logger->log(LogLevel::Info,
				"abort http connection from:", self->clientAddress_, "because of", ex);
		}).then([self] {
			// cancel timer anyway in case of connection error
			self->shutdownInputTimer_.cancel();
			// remove self from connections collection (notice it's weak_ptr)
			auto* connectionsPtr = self->sharedData_->connectionsWrapper.get();
			std::size_t connectionsCount = 0;
			if (connectionsPtr != nullptr) {
				connectionsPtr->value.erase(self);
				connectionsCount = connectionsPtr->value.size();
				self->sharedData_->metricData.current_connections = connectionsCount;
			}
			// log and update state to closed
			self->sharedData_->logger->log(LogLevel::Info,
				"close http connection from:", self->clientAddress_,
				"(count:", connectionsCount, ")");
			self->state_ = Http11ServerConnectionState::Closed;
		});
	}
	
	/** Stop the connection immediately */
	seastar::future<> Http11ServerConnection::stop() {
		// update state to closing
		state_ = Http11ServerConnectionState::Closing;
		// abort reader and writer
		if (socket_.isConnected()) {
			socket_.socket().shutdown_output();
			socket_.socket().shutdown_input();
		}
		// wait until connection closed
		// check state every seconds instead of allocate promise object,
		// because stop connection is a rare operation
		auto self = shared_from_this();
		return seastar::do_until(
			[self] { return self->state_ == Http11ServerConnectionState::Closed; },
			[] { return seastar::sleep(std::chrono::seconds(1)); });
	}
	
	/** Constructor */
	Http11ServerConnection::Http11ServerConnection(
		const seastar::lw_shared_ptr<HttpServerSharedData>& sharedData,
		seastar::connected_socket&& fd,
		seastar::socket_address&& addr) :
		sharedData_(sharedData),
		socket_(std::move(fd)),
		clientAddress_(std::move(addr)),
		state_(Http11ServerConnectionState::Initial),
		request_(),
		response_(),
		parser_(),
		temporaryData_(),
		nextRequestBuffer_(),
		shutdownInputTimer_() {
		// initialize timer
		shutdownInputTimer_.set_callback([this] {
			sharedData_->metricData.request_timeout_errors += 1;
			sharedData_->logger->log(LogLevel::Info,
				"abort http connection from:", clientAddress_,
				"because of initial request timeout");
			socket_.socket().shutdown_input();
		});
	}
	
	/** Receive headers from single request, the body may not completely received */
	seastar::future<> Http11ServerConnection::receiveSingleRequest() {
		if (state_ >= Http11ServerConnectionState::ReceiveRequestHeadersComplete) {
			// either headers completed or connection closing
			return seastar::make_ready_future<>();
		}
		// receive request headers
		// use custom timer instead of seastar::with_timeout for following reasons:
		// - with_timeout will allocate a new timer and the callback function each time
		// - delete connected_socket before read operation is finished will cause use-after-delete error
		shutdownInputTimer_.arm(seastar::timer<>::clock::now() +
			sharedData_->configuration.getInitialRequestTimeout());
		seastar::future f = (nextRequestBuffer_.size() == 0 ?
			socket_.in().read() :
			seastar::make_ready_future<seastar::temporary_buffer<char>>(std::move(nextRequestBuffer_)));
		return std::move(f).then([this] (seastar::temporary_buffer<char> tempBuffer) {
			// cancel timer
			shutdownInputTimer_.cancel();
			// store the last buffer received
			auto& lastBuffer = temporaryData_.lastBuffer;
			lastBuffer = std::move(tempBuffer);
			// check whether connection is closed from remote
			if (lastBuffer.size() == 0) {
				state_ = Http11ServerConnectionState::Closing;
				return seastar::make_ready_future<>();
			}
			// check bytes limitation of initial request data
			// no overflow check of receivedBytes because the buffer size should be small (up to 8192)
			// if receivedBytes + buffer size cause overflow that mean the limitation is too large
			temporaryData_.receivedBytes += lastBuffer.size();
			if (CPV_UNLIKELY(temporaryData_.receivedBytes >
				sharedData_->configuration.getMaxInitialRequestBytes())) {
				sharedData_->metricData.request_initial_size_errors += 1;
				return replyStaticResponse(socket_, ReachedBytesLimitationOfInitialRequestData).then([] {
					return seastar::make_exception_future<>(LengthException(
						CPV_CODEINFO, "http request length error:",
						"reached bytes limitation of initial request data"));
				});
			}
			// check limitation of received packets, to avoid small packet attack
			temporaryData_.receivedPackets += 1;
			if (CPV_UNLIKELY(temporaryData_.receivedPackets >
				sharedData_->configuration.getMaxInitialRequestPackets())) {
				sharedData_->metricData.request_initial_size_errors += 1;
				return replyStaticResponse(socket_, ReachedPacketsLimitationOfInitialRequestData).then([] {
					return seastar::make_exception_future<>(LengthException(
						CPV_CODEINFO, "http request length error:",
						"reached packets limitation of initial request data"));
				});
			}
			// execute http parser
			std::size_t parsedSize = internal::http_parser::http_parser_execute(
				&parser_,
				this,
				lastBuffer.get(),
				lastBuffer.size());
			if (parsedSize != lastBuffer.size()) {
				auto err = static_cast<enum internal::http_parser::http_errno>(parser_.http_errno);
				if (CPV_LIKELY(err == internal::http_parser::http_errno::HPE_CB_message_begin &&
					parsedSize > 1 && temporaryData_.messageCompleted)) {
					// received next request from pipeline
					// the http parser will become error state,
					// but it will reinitialize before processing next request
					nextRequestBuffer_ = lastBuffer.share();
					nextRequestBuffer_.trim_front(parsedSize - 1);
				} else {
					// parse error
					return replyErrorResponseForInvalidFormat();
				}
			}
			// hold underlying buffer in request
			request_.addUnderlyingBuffer(std::move(lastBuffer));
			// continue receiving
			return receiveSingleRequest();
		});
	}
	
	/** Reply single response and ensure the request body is completely received */
	seastar::future<> Http11ServerConnection::replySingleResponse() {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete ||
			state_ == Http11ServerConnectionState::ReceiveRequestBody ||
			state_ == Http11ServerConnectionState::ReceiveRequestMessageComplete) {
			// atleast all headers received, start replying response
			state_ = Http11ServerConnectionState::ReplyResponse;
			// setup request and response stream
			request_.setBodyStream(
				makeObject<Http11ServerConnectionRequestStream>(this).cast<InputStreamBase>());
			response_.setBodyStream(
				makeObject<Http11ServerConnectionResponseStream>(this).cast<OutputStreamBase>());
			// call the first handler
			return sharedData_->handlers.front()->handle(
				request_, response_, sharedData_->handlers.begin() + 1).then([this] {
				// send response headers if not sent befofe, then flush response
				seastar::future<> result = seastar::make_ready_future<>();
				if (CPV_LIKELY(temporaryData_.responseHeadersAppended)) {
					result = socket_.out().flush();
				} else {
					seastar::net::packet data(getResponseHeadersFragmentsCount());
					appendResponseHeaders(data);
					result = socket_.out().put(std::move(data)).then([this] {
						return socket_.out().flush();
					});
				}
				// determine whether should close the connection after response is end
				// should call appendResponseHeaders before reach here
				if (CPV_LIKELY(temporaryData_.keepConnection)) {
					temporaryData_.keepConnection = shouldKeepConnectionForContentLength(
						response_, temporaryData_.responseBodyWrittenSize,
						temporaryData_.messageCompleted, sharedData_->logger, clientAddress_);
				}
				if (CPV_UNLIKELY(!temporaryData_.keepConnection)) {
					state_ = Http11ServerConnectionState::Closing;
				}
				return result;
			});
		} else if (state_ == Http11ServerConnectionState::Closing) {
			// connection closing
			return seastar::make_ready_future<>();
		} else {
			// state error
			return replyStaticResponse(socket_, InvalidStateAfterReceivedSingleRequest).then([this] {
				return seastar::make_exception_future<>(LogicException(
					CPV_CODEINFO, "invalid state after received single request:", state_));
			});
		}
	}
	
	/** Get how many fragments should reserve for response headers, may greater than actual count  */
	std::size_t Http11ServerConnection::getResponseHeadersFragmentsCount() const {
		// calculate fragments count
		// +6: version, space, status code, space, status message, crlf
		// +4: date header, colon + space, header value, crlf
		// +4: server header, colon + space, header value, crlf
		// +4: connection header, colon + space, header value, crlf
		// + headers count * 4
		//   reserve 3 addition header for date, server, connection
		// +1: crlf
		return 31 + response_.getHeaders().size() * 4;
	}
	
	/** Append response headers to packet, please check responseHeadersAppended first */
	void Http11ServerConnection::appendResponseHeaders(seastar::net::packet& packet) {
		// check flag
		if (CPV_UNLIKELY(temporaryData_.responseHeadersAppended)) {
			return;
		}
		temporaryData_.responseHeadersAppended = true;
		// determine response http protocol version
		std::string_view version = response_.getVersion();
		if (CPV_LIKELY(version.empty())) {
			// copy version from request
			version = request_.getVersion();
			if (CPV_UNLIKELY(version.empty())) {
				// request version is unsupported
				version = constants::Http10;
			}
			// store version for keepalive determination later
			response_.setVersion(version);
		}
		// return a special status code for handler didn't set it
		if (CPV_UNLIKELY(
			response_.getStatusCode().empty() ||
			response_.getStatusMessage().empty())) {
			response_.setStatusCode("0");
			response_.setStatusMessage("Status code or status message not set");
		}
		// determine value of date header
		std::string_view date;
		auto& headers = response_.getHeaders();
		auto dateIt = headers.find(constants::Date);
		if (CPV_LIKELY(dateIt == headers.end())) {
			date = formatNowForHttpHeader();
		} else {
			date = dateIt->second;
			headers.erase(dateIt);
		}
		// determine value of server header
		std::string_view server;
		auto serverIt = headers.find(constants::Server);
		if (CPV_LIKELY(serverIt == headers.end())) {
			// no version number for security
			server = constants::CPVFramework;
		} else {
			server = serverIt->second;
			headers.erase(serverIt);
		}
		// determine value of connection header
		std::string_view connection;
		temporaryData_.keepConnection = shouldKeepConnectionForConnectionHeader(request_, version);
		auto connectionIt = headers.find(constants::Connection);
		if (CPV_LIKELY(connectionIt == headers.end())) {
			if (CPV_LIKELY(temporaryData_.keepConnection)) {
				connection = constants::Keepalive;
			} else {
				connection = constants::Close;
			}
		} else {
			// custom connection header, close connection if it isn't keep-alive
			if (CPV_LIKELY(connection != constants::Keepalive)) {
				temporaryData_.keepConnection = false;
			}
			connection = connectionIt->second;
			headers.erase(connectionIt);
		}
		// append response headers to packet
		packet << version << constants::Space <<
			response_.getStatusCode() << constants::Space <<
			response_.getStatusMessage() << constants::CRLF;
		packet << constants::Date << constants::ColonSpace <<
			date << constants::CRLF;
		packet << constants::Server << constants::ColonSpace <<
			server << constants::CRLF;
		packet << constants::Connection << constants::ColonSpace <<
			connection << constants::CRLF;
		for (auto& pair : response_.getHeaders()) {
			packet << pair.first << constants::ColonSpace <<
				pair.second << constants::CRLF;
		}
		packet << constants::CRLF;
	}
	
	/** Reply error response for invalid http request format, then return exception future */
	seastar::future<> Http11ServerConnection::replyErrorResponseForInvalidFormat() {
		sharedData_->metricData.request_invalid_format_errors += 1;
		return replyStaticResponse(socket_, InvalidHttpRequestFormat).then([this] {
			auto err = static_cast<enum internal::http_parser::http_errno>(parser_.http_errno);
			return seastar::make_exception_future<>(FormatException(
				CPV_CODEINFO, "http request format error:",
				internal::http_parser::http_errno_name(err),
				internal::http_parser::http_errno_description(err),
				", state:", state_));
		});
	}
	
	int Http11ServerConnection::on_message_begin() {
		if (state_ == Http11ServerConnectionState::ReceiveRequestInitial) {
			// normal begin
			state_ = Http11ServerConnectionState::ReceiveRequestMessageBegin;
		} else {
			// state error, maybe the next request from pipeline,
			// the caller should check parser_.http_errno and remember rest of the buffer
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_url(const char* data, std::size_t size) {
		if (state_ == Http11ServerConnectionState::ReceiveRequestMessageBegin) {
			// the first time received the url
			state_ = Http11ServerConnectionState::ReceiveRequestUrl;
			temporaryData_.urlView = std::string_view(data, size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// url splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				temporaryData_.urlMerged,
				temporaryData_.urlView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_header_field(const char* data, std::size_t size) {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// the first time received a new header field, flush last header field and value
			state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
			flushMergedTemporaryBuffer(request_, temporaryData_.headerFieldMerged);
			flushMergedTemporaryBuffer(request_, temporaryData_.headerValueMerged);
			request_.setHeader(
				temporaryData_.headerFieldView,
				temporaryData_.headerValueView);
			temporaryData_.headerFieldView = std::string_view(data, size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// the first time received the first header field
			state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
			temporaryData_.headerFieldView = std::string_view(data, size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderField) {
			// header field splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				temporaryData_.headerFieldMerged,
				temporaryData_.headerFieldView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_header_value(const char* data, std::size_t size) {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderField) {
			// the first time received a header value after header field
			state_ = Http11ServerConnectionState::ReceiveRequestHeaderValue;
			temporaryData_.headerValueView = std::string_view(data, size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// header value splited in multiple packets, merge them to a temporary buffer
			mergeContent(
				temporaryData_.headerValueMerged,
				temporaryData_.headerValueView,
				std::string_view(data, size));
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_headers_complete() {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
			// all headers received, flush last header field and value
			state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
			flushMergedTemporaryBuffer(request_, temporaryData_.headerFieldMerged);
			flushMergedTemporaryBuffer(request_, temporaryData_.headerValueMerged);
			request_.setHeader(
				temporaryData_.headerFieldView,
				temporaryData_.headerValueView);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
			// no headers but url
			state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
		} else {
			// state error
			return -1;
		}
		// flush method, url and version
		flushMergedTemporaryBuffer(request_, temporaryData_.urlMerged);
		request_.setMethod(internal::http_parser::http_method_str(
			static_cast<enum internal::http_parser::http_method>(parser_.method)));
		request_.setUrl(temporaryData_.urlView);
		request_.setVersion(getHttpVersionString(parser_));
		return 0;
	}
	
	int Http11ServerConnection::on_body(const char* data, std::size_t size) {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete) {
			// received initial body, share lastBuffer to bodyBuffer
			auto& lastBuffer = temporaryData_.lastBuffer;
			state_ = Http11ServerConnectionState::ReceiveRequestBody;
			temporaryData_.bodyBuffer = lastBuffer.share(data - lastBuffer.get(), size);
		} else if (state_ == Http11ServerConnectionState::ReceiveRequestBody) {
			// received initial chunked body, share lastBuffer to moreBodyBuffers
			auto& lastBuffer = temporaryData_.lastBuffer;
			temporaryData_.moreBodyBuffers.emplace_back(
				lastBuffer.share(data - lastBuffer.get(), size));
		} else if (state_ == Http11ServerConnectionState::ReplyResponse) {
			// receive body when replying response (called from request stream)
			auto& bodyBuffer = temporaryData_.bodyBuffer;
			if (bodyBuffer.empty()) {
				// move lastBuffer to bodyBuffer
				bodyBuffer = std::move(temporaryData_.lastBuffer);
				bodyBuffer.trim_front(data - bodyBuffer.get());
				bodyBuffer.trim(size);
			} else {
				// share bodyBuffer to moreBodyBuffers
				temporaryData_.moreBodyBuffers.emplace_back(
					bodyBuffer.share(data - bodyBuffer.get(), size));
			}
		} else {
			// state error
			return -1;
		}
		return 0;
	}
	
	int Http11ServerConnection::on_message_complete() {
		if (state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete ||
			state_ == Http11ServerConnectionState::ReceiveRequestBody) {
			// no body or all body is received before reply
			state_ = Http11ServerConnectionState::ReceiveRequestMessageComplete;
			temporaryData_.messageCompleted = true;
		} else if (state_ == Http11ServerConnectionState::ReplyResponse) {
			// all body received when replying response (called from request stream)
			temporaryData_.messageCompleted = true;
		} else {
			// state error
			return -1;
		}
		return 0;
	}
}

