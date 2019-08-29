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
		
		/** Parser callbacks */
		int on_message_begin();
		int on_url(const char*, size_t);
		using internal::http_parser::http_parser_settings::on_status;
		int on_header_field(const char*, size_t);
		int on_header_value(const char*, size_t);
		int on_headers_complete();
		int on_body(const char*, size_t);
		int on_message_complete();
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
		struct BodyEntry { seastar::temporary_buffer<char> buffer; std::uint32_t id; bool isEnd; };
		seastar::queue<BodyEntry> requestBodyQueue_;
		// the new request, when headers completed it will move to requestQueue_
		HttpRequest newRequest_;
		// the rest of buffer for next request received from pipeline
		seastar::temporary_buffer<char> nextRequestBuffer_;
		// the http context handling now
		HttpContext processingContext_;
		// the error response send to client before close connection
		// usually it's cause by invalid format or headers too large
		std::string_view lastErrorResponse_;
		// the shutdown reason used for logging
		const char* shutdownReason_;
		// the parser used to parse http request
		internal::http_parser::http_parser parser_;
		// per request data for receive loop, use unnamed struct for fast reset
		struct {
			// new request id (only for internal error check)
			std::uint32_t requestId = 0;
			// the last buffer received, store from receiveSingleRequest or request stream
			seastar::temporary_buffer<char> lastBuffer;
			// for initial request size check
			std::size_t receivedBytes = 0;
			std::size_t receivedPackets = 0;
			// for url splited in multiple packets
			seastar::temporary_buffer<char> urlMerged;
			std::string_view urlView;
			// for header field splited in multiple packets
			seastar::temporary_buffer<char> headerFieldMerged;
			std::string_view headerFieldView;
			// for header value splited in multiple packets
			seastar::temporary_buffer<char> headerValueMerged;
			std::string_view headerValueView;
			// for body, may splited as multiple parts if encoding is chunked
			StackAllocatedVector<seastar::temporary_buffer<char>, 3> bodyBuffers;
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

