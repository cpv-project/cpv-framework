#pragma once
#include <CPVFramework/Http/Client/HttpClientRequest.hpp>

namespace cpv {
	/** Implementation of http request builder */
	class HttpClientRequestImpl : public HttpClientRequest {
	public:
		/** For Object */
		void reset(
			const std::string_view& method,
			const std::string_view& path,
			const std::string_view& host);

		/** For Object */
		static void freeResources();

		/** Add a header */
		void addHeader(
			const std::string_view& key,
			const std::string_view& value) override;

		/** Set the body of this request */
		void setBody(
			const std::string_view& mimeType,
			const std::string_view& content) override;

		/** Get the full content of this request */
		const std::string_view str() const& override;

		/** Constructor */
		HttpClientRequestImpl();

	private:
		enum class State { Initial = 0, HeaderFinished = 1 };
		State state_;
		std::string str_;
	};
}

