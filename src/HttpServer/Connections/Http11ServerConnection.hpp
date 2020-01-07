#pragma once
#include <optional>
#include <seastar/core/queue.hh>
#include <CPVFramework/Allocators/StackAllocator.hpp>
#include <CPVFramework/Utility/EnumUtils.hpp>
#include <CPVFramework/Utility/SocketHolder.hpp>
#include <CPVFramework/HttpServer/HttpServerConfiguration.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestHandlerBase.hpp>
#include "../HttpServerSharedData.hpp"
#include "./HttpServerConnectionBase.hpp"
#include "./Http11Parser.hpp"

namespace cpv {
	/** The state of a http 1.0/1.1 connection, only for receive loop */
	enum class Http11ServerConnectionState {
		Initial,
		Started,
		ReceiveRequestMessageBegin,
		ReceiveRequestUrl,
		ReceiveRequestHeaderField,
		ReceiveRequestHeaderValue,
		ReceiveRequestHeadersComplete,
		ReceiveRequestBody,
		ReceiveRequestMessageComplete,
		Closing,
		Closed
	};
	
	/** Enum descriptions of Http11ServerConnectionState */
	template <>
	struct EnumDescriptions<Http11ServerConnectionState> {
		static const std::vector<std::pair<Http11ServerConnectionState, const char*>>& get();
	};
	
	/** Connection accepted from http client uses http 1.0/1.1 protocol */
	class Http11ServerConnection :
		public HttpServerConnectionBase,
		private internal::http_parser::http_parser_settings,
		public seastar::enable_shared_from_this<Http11ServerConnection> {
	public:
		/** Start receive requests and send responses */
		void start();
		
		/** Stop the connection immediately */
		seastar::future<> stop() override;
		
		/** Invoke when timeout is detected from HttpServer's timer */
		void onTimeout() override;
		
		/** Constructor */
		Http11ServerConnection(
			const seastar::lw_shared_ptr<HttpServerSharedData>& sharedData,
			seastar::connected_socket&& fd,
			seastar::socket_address&& addr);
		
	private:
		/** Shutdown connection, break receive and reply loop */
		void shutdown(const char* reason);
		
		/** start receiveRequestLoop and catch exceptions */
		seastar::future<> startReceiveRequestLoop();
		
		/** Keep receiving requests until state become closing, will push requests to a queue */
		seastar::future<> receiveRequestLoop();

		/** (for receive loop) Enqueue body buffers to queue */
		seastar::future<> enqueueBodyBuffers();
		
		/** (for receive loop) Enqueue request to queue when headers completed, and continue receiving */
		seastar::future<> enqueueRequestAndContinueReceiving(bool hasBody);
		
		/** start replyResponseLoop and catch exceptions */
		seastar::future<> startReplyResponseLoop();
		
		/** Keep reply responses until state become closing, will pop requests from the queue */
		seastar::future<> replyResponseLoop();
		
		/** (for reply loop) Get maximum fragments count for response headers */
		std::size_t getResponseHeadersFragmentsCount() const;
		
		/** (for reply loop) Append response headers to packet, please check responseHeadersAppended first */
		void appendResponseHeaders(Packet& packet);
		
		/** (for reply loop) Determine whether keep connection or not by checking connection header */
		bool checkKeepaliveByConnnectionHeader() const;
		
		/** (for reply loop) Determine whether keep connection or not by checking content length */
		bool checkKeepaliveByContentLength() const;
		
	private:
		/** Parser callback */
		CPV_INLINE int on_message_begin() {
			if (CPV_LIKELY(state_ == Http11ServerConnectionState::Started)) {
				// normal begin
				state_ = Http11ServerConnectionState::ReceiveRequestMessageBegin;
			} else {
				// state error, maybe the next request from pipeline,
				// the caller should check parser_.http_errno and remember rest of the buffer
				return -1;
			}
			return 0;
		}
		
		/** Parser callback */
		CPV_INLINE int on_url(const char* data, size_t size) {
			if (CPV_LIKELY(state_ == Http11ServerConnectionState::ReceiveRequestMessageBegin)) {
				// the first time received the url
				state_ = Http11ServerConnectionState::ReceiveRequestUrl;
				receiveLoopData_.url = SharedStringBuilder(
					receiveLoopData_.lastBuffer.share({ data, size }));
			} else if (state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
				// url splited in multiple packets, merge them
				receiveLoopData_.url.append({ data, size });
			} else {
				// state error
				return -1;
			}
			return 0;
		}
		
		/** Parser callback */
		CPV_INLINE int on_header_field(const char* data, size_t size) {
			if (CPV_LIKELY(state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue)) {
				// the first time received a new header field, store last header field and value
				state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
				newRequest_.setHeader(
					receiveLoopData_.headerField.build(),
					receiveLoopData_.headerValue.build());
				receiveLoopData_.headerField = SharedStringBuilder(
					receiveLoopData_.lastBuffer.share({ data, size }));
			} else if (CPV_LIKELY(state_ == Http11ServerConnectionState::ReceiveRequestUrl)) {
				// the first time received the first header field
				state_ = Http11ServerConnectionState::ReceiveRequestHeaderField;
				receiveLoopData_.headerField = SharedStringBuilder(
					receiveLoopData_.lastBuffer.share({ data, size }));
			} else if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderField) {
				// header field splited in multiple packets, merge them
				receiveLoopData_.headerField.append({ data, size });
			} else {
				// state error
				return -1;
			}
			return 0;
		}
		
		/** Parser callback */
		CPV_INLINE int on_header_value(const char* data, size_t size) {
			if (CPV_LIKELY(state_ == Http11ServerConnectionState::ReceiveRequestHeaderField)) {
				// the first time received a header value after header field
				state_ = Http11ServerConnectionState::ReceiveRequestHeaderValue;
				receiveLoopData_.headerValue = SharedStringBuilder(
					receiveLoopData_.lastBuffer.share({ data, size }));
			} else if (state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue) {
				// header value splited in multiple packets, merge them
				receiveLoopData_.headerValue.append({ data, size });
			} else {
				// state error
				return -1;
			}
			return 0;
		}
		
		/** Parser callback */
		CPV_INLINE int on_headers_complete() {
			if (CPV_LIKELY(state_ == Http11ServerConnectionState::ReceiveRequestHeaderValue)) {
				// all headers received, store last header field and value
				state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
				newRequest_.setHeader(
					receiveLoopData_.headerField.build(),
					receiveLoopData_.headerValue.build());
			} else if (state_ == Http11ServerConnectionState::ReceiveRequestUrl) {
				// no headers but url
				state_ = Http11ServerConnectionState::ReceiveRequestHeadersComplete;
			} else {
				// state error
				return -1;
			}
			// store method, url and version
			newRequest_.setMethod(SharedString::fromStatic(
				internal::http_parser::http_method_str(
					static_cast<enum internal::http_parser::http_method>(parser_.method))));
			newRequest_.setUrl(receiveLoopData_.url.build());
			newRequest_.setVersion(getHttpVersionString(parser_));
			return 0;
		}
		
		/** Parser callback */
		CPV_INLINE int on_body(const char* data, size_t size) {
			// warning:
			// newRequest_ maybe empty (moved to queue but not all completed), don't touch it
			if (state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete) {
				// received initial body, share lastBuffer to bodyBuffers
				state_ = Http11ServerConnectionState::ReceiveRequestBody;
				receiveLoopData_.bodyBuffers.emplace_back(
					receiveLoopData_.lastBuffer.share({ data, size }));
			} else if (state_ == Http11ServerConnectionState::ReceiveRequestBody) {
				// received following body, share lastBuffer to bodyBuffers
				receiveLoopData_.bodyBuffers.emplace_back(
					receiveLoopData_.lastBuffer.share({ data, size }));
			} else {
				// state error
				return -1;
			}
			return 0;
		}
		
		/** Parser callback */
		CPV_INLINE int on_message_complete() {
			// warning:
			// newRequest_ maybe empty (moved to queue but not all completed), don't touch it
			if (CPV_LIKELY(
				state_ == Http11ServerConnectionState::ReceiveRequestHeadersComplete ||
				state_ == Http11ServerConnectionState::ReceiveRequestBody)) {
				// no body or all body received
				if (receiveLoopData_.requestEnqueued && receiveLoopData_.bodyBuffers.empty()) {
					// request enqueued before completed, but no body received
					// add empty tail body to make sure stream can finish
					receiveLoopData_.bodyBuffers.emplace_back();
				}
				state_ = Http11ServerConnectionState::ReceiveRequestMessageComplete;
			} else {
				// state error
				return -1;
			}
			return 0;
		}
		
		/** Get http version string from parser */
		CPV_INLINE static SharedString getHttpVersionString(
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
			return SharedStringBuilder(9)
				.append("HTTP/")
				.append(parser.http_major)
				.append(".")
				.append(parser.http_minor)
				.build();
		}
		
		using internal::http_parser::http_parser_settings::on_status;
		using internal::http_parser::http_parser_settings::on_chunk_header;
		using internal::http_parser::http_parser_settings::on_chunk_complete;
		
		/** Friends **/
		friend size_t internal::http_parser::http_parser_execute<>(
			http_parser*, Http11ServerConnection*, const char*, size_t);
		friend class Http11ServerConnectionRequestStream;
		friend class Http11ServerConnectionResponseStream;
		
	private:
		seastar::lw_shared_ptr<HttpServerSharedData> sharedData_;
		SocketHolder socket_;
		Http11ServerConnectionState state_;
		// the queue store received requests which headers completed (notice body may not completed)
		struct RequestEntry { HttpRequest request; std::uint32_t id; bool hasBody; };
		seastar::queue<RequestEntry> requestQueue_;
		// the queue store received body buffers for all requests
		struct BodyEntry { SharedString buffer; std::uint32_t id; bool isEnd; };
		seastar::queue<BodyEntry> requestBodyQueue_;
		// the new request, when headers completed it will move to requestQueue_
		HttpRequest newRequest_;
		// the rest of buffer for next request received from pipeline
		seastar::temporary_buffer<char> nextRequestBuffer_;
		// the http context handling now
		HttpContext processingContext_;
		// the error response send to client before close connection
		// usually it's cause by invalid format or headers too large
		SharedString lastErrorResponse_;
		// the shutdown reason used for logging
		const char* shutdownReason_;
		// the parser used to parse http request
		internal::http_parser::http_parser parser_;
		// per request data for receive loop, use unnamed struct for fast reset
		struct {
			// new request id (only for internal error check)
			std::uint32_t requestId = 0;
			// the last buffer received, store from receiveSingleRequest or request stream
			SharedString lastBuffer;
			// for initial request size check
			std::size_t receivedBytes = 0;
			std::size_t receivedPackets = 0;
			// for url that may splited in multiple packets
			SharedStringBuilder url;
			// for header field that may splited in multiple packets
			SharedStringBuilder headerField;
			// for header value that may splited in multiple packets
			SharedStringBuilder headerValue;
			// for body, may splited as multiple parts if encoding is chunked
			StackAllocatedVector<SharedString, 3> bodyBuffers;
			std::size_t bodyBufferEnqueueIndex = 0;
			// is newRequest_ enqueued (empty now)
			bool requestEnqueued = false;
		} receiveLoopData_;
		// per request and response data for reply loop, use unnamed struct for fast reset
		struct {
			//.processing request id (only for internal error check)
			std::uint32_t requestId = 0;
			// is processing request body consumed
			bool requestBodyConsumed = false;
			// is response headers appended to packet previously
			bool responseHeadersAppended = false;
			// bytes of response body written to client
			std::size_t responseWrittenBytes = 0;
			// is connection keeping for next request
			bool keepConnection = false;
		} replyLoopData_;
	};
}

