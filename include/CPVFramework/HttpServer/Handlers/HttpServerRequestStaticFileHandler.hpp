#pragma once
#include "../../Utility/SharedString.hpp"
#include "./HttpServerRequestHandlerBase.hpp"

namespace cpv {
	/** Members of HttpServerRequestStaticFileHandler */
	class HttpServerRequestStaticFileHandlerData;

	/**
	 * Request handler that returns content of files.
	 *
	 * It will check whether url is starts with urlBase, and check whether file path is exists.
	 * For example:
	 * urlBase is /static, pathBase is ./storage/static
	 * When request url is /static/js/main.js, this handler will try to return ./storage/static/js/main.js.
	 * Notice if path is not exists or contains invalid chars like '..' or '//' then it will pass to next handler.
	 *
	 * It can use lru cache to cache file content in memory (per cpu core), if file size is less than
	 * maxCacheFileSize and maxCacheFileEntities is not 0, it will put file content to cache for next use.
	 * You should disable file caching for local development environment by setting maxCacheFileEntities to 0.
	 * 
	 * It supports pre-compressed gzip files, for example if file path is ./1.txt and client accept gzip
	 * encoding, then it will also search for ./1.txt.gz and return it if exists.
	 *
	 * It supports Range header, but it won't use cache and gzip files when range header is presented,
	 * because usually range header is used for downloading large pre-compressed files.
	 *
	 * It supports If-Modified-Since header, if file not change then it will return 304 response.
	 */
	class HttpServerRequestStaticFileHandler : public HttpServerRequestHandlerBase {
	public:
		static const std::size_t DefaultMaxCacheFileEntities = 16;
		static const std::size_t DefaultMaxCacheFileSize = 1048576; // 1mb

		/** Return content of request file */
		seastar::future<> handle(
			HttpContext& context,
			HttpServerRequestHandlerIterator next) const override;

		/** Clear cached file contents */
		void clearCache();

		/** Constructor */
		HttpServerRequestStaticFileHandler(
			// like "/static"
			SharedString&& urlBase,
			// like "./storage/static"
			SharedString&& pathBase,
			// like "max-age=84600, public" or "" (not sending Cache-Control)
			SharedString&& cacheControl = "",
			std::size_t maxCacheFileEntities = DefaultMaxCacheFileEntities,
			std::size_t maxCacheFileSize = DefaultMaxCacheFileSize);

		/** Move constructor (for incomplete member type) */
		HttpServerRequestStaticFileHandler(HttpServerRequestStaticFileHandler&&);

		/** Move assign operator (for incomplete member type) */
		HttpServerRequestStaticFileHandler& operator=(HttpServerRequestStaticFileHandler&&);

		/** Destructor (for incomplete member type) */
		~HttpServerRequestStaticFileHandler();

	private:
		std::unique_ptr<HttpServerRequestStaticFileHandlerData> data_;
	};
}

