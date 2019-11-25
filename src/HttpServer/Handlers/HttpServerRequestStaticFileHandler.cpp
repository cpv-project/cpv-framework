#include <array>
#include <seastar/core/reactor.hh>
#include <seastar/core/fstream.hh>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Allocators/StackAllocator.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestStaticFileHandler.hpp>
#include <CPVFramework/Http/HttpConstantStrings.hpp>
#include <CPVFramework/Http/HttpResponseExtensions.hpp>
#include <CPVFramework/Utility/DateUtils.hpp>
#include <CPVFramework/Utility/FileUtils.hpp>
#include <CPVFramework/Utility/HttpUtils.hpp>
#include <CPVFramework/Utility/LRUCache.hpp>
#include <CPVFramework/Utility/Macros.hpp>
#include <CPVFramework/Utility/StringUtils.hpp>

namespace cpv {
	/** Members of HttpServerRequestStaticFileHandler */
	class HttpServerRequestStaticFileHandlerData {
	public:
		/** File content and modified time */
		struct FileCacheEntry {
			SharedString content;
			SharedString lastModified;

			/** Constructor */
			FileCacheEntry(
				SharedString&& contentVal,
				SharedString&& lastModifiedVal) :
				content(std::move(contentVal)),
				lastModified(std::move(lastModifiedVal)) { }

			/** Reply to http response with 304 or 200 */
			seastar::future<> reply(
				HttpResponse& response,
				SharedString&& mimeType,
				SharedString&& ifModifiedSinceHeader,
				const SharedString& cacheControl) {
				// set Cache-Control if present
				auto& headers = response.getHeaders();
				if (!cacheControl.empty()) {
					headers.setCacheControl(cacheControl.share());
				}
				// check if we can return 304 not modified
				if (ifModifiedSinceHeader == lastModified) {
					response.setStatusCode(constants::_304);
					response.setStatusMessage(constants::NotModified);
					headers.setContentType(std::move(mimeType));
					headers.setLastModified(std::move(ifModifiedSinceHeader));
					return seastar::make_ready_future<>();
				}
				// return cached file content
				headers.setLastModified(lastModified.share());
				return extensions::reply(response, content.share(), std::move(mimeType));
			}
		};

		SharedString urlBase;
		SharedString pathBase;
		SharedString cacheControl;
		std::size_t maxCacheFileSize;
		LRUCache<SharedString, FileCacheEntry> fileCache;

		/** Constructor */
		HttpServerRequestStaticFileHandlerData(
			SharedString&& urlBaseVal,
			SharedString&& pathBaseVal,
			SharedString&& cacheControlVal,
			std::size_t maxCacheFileEntitiesVal,
			std::size_t maxCacheFileSizeVal) :
			urlBase(std::move(urlBaseVal)),
			pathBase(std::move(pathBaseVal)),
			cacheControl(std::move(cacheControlVal)),
			maxCacheFileSize(maxCacheFileSizeVal),
			fileCache(maxCacheFileEntitiesVal) {
			urlBase.trim(trimString<false, true>(urlBase, "/\\"));
			pathBase.trim(trimString<false, true>(urlBase, "/\\"));
		}
	};

	namespace {
		/** Suffix of gzip compressed file */
		static const constexpr char CompressedFileSuffix[] = ".gz";
		/** Encoding string of gzip compressed content */
		static const constexpr char CompressEncoding[] = "gzip";
		/** Prefix for Range header, only bytes is supported */
		static const constexpr char RangePrefix[] = "bytes=";

		/** Class for reply content of static file, allocate in once */
		class StaticFileReplier {
		public:
			/** Execute reply operation */
			seastar::future<> execute() {
				auto& path = getPath();
				return seastar::engine().file_type(seastar::sstring(path.data(), path.size()))
				.then([this] (auto type) {
					if (!type || *type == seastar::directory_entry_type::directory) {
						// file not exists or is directory
						if (supportCompress_) {
							supportCompress_ = false;
							return execute();
						} else {
							return (*next_)->handle(context_, next_ + 1);
						}
					}
					auto& path = getPath();
					return seastar::open_file_dma(
						seastar::sstring(path.data(), path.size()), seastar::open_flags::ro)
					.then([this] (seastar::file f) {
						file_ = std::move(f);
						return file_.stat();
					}).then([this] (struct ::stat st) {
						// set Cache-Control if present
						auto& response = context_.getResponse();
						auto& headers = response.getHeaders();
						if (!data_.cacheControl.empty()) {
							headers.setCacheControl(data_.cacheControl.share());
						}
						// check if we can return 304 not modified
						std::string_view lastModifiedStr = formatTimeForHttpHeader(st.st_mtime);
						if (ifModifiedSinceHeader_ == lastModifiedStr) {
							response.setStatusCode(constants::_304);
							response.setStatusMessage(constants::NotModified);
							headers.setContentType(std::move(mimeType_));
							headers.setLastModified(std::move(ifModifiedSinceHeader_));
							return seastar::make_ready_future<>();
						}
						// allocate string for Last-Modified header
						SharedString lastModified(lastModifiedStr);
						// set Content-Encoding if compressed file is used
						if (supportCompress_) {
							headers.setContentEncoding(CompressEncoding);
						}
						// read whole file from disk and store to cache if appropriate
						fileStream_ = seastar::make_file_input_stream(std::move(file_));
						std::size_t fileSize = static_cast<std::size_t>(st.st_size);
						if (fileSize <= data_.maxCacheFileSize &&
							data_.fileCache.maxSize() > 0 && rangeHeader_.empty()) {
							return fileStream_.read_exactly(fileSize).then(
								[this, lastModified=std::move(lastModified)] (auto buf) mutable {
								// store file content to cache
								data_.fileCache.set(
									getPath().share(), getPath().share(),
									{ buf.share(), lastModified.share() });
								// set Last-Modified header
								auto& response = context_.getResponse();
								auto& headers = response.getHeaders();
								headers.setLastModified(std::move(lastModified));
								return extensions::reply(response,
									std::move(buf), std::move(mimeType_));
							});
						}
						// TODO
						return seastar::make_ready_future<>();
					});
				});
			}

			/** Constructor */
			StaticFileReplier(
				SharedString&& filePath,
				SharedString&& compressedPath,
				bool supportCompress,
				SharedString&& rangeHeader,
				SharedString&& mimeType,
				SharedString&& ifModifiedSinceHeader,
				HttpServerRequestStaticFileHandlerData& data,
				HttpContext& context,
				HttpServerRequestHandlerIterator next) :
				filePath_(std::move(filePath)),
				compressedPath_(std::move(compressedPath)),
				supportCompress_(supportCompress),
				rangeHeader_(std::move(rangeHeader)),
				range_(parseRangeHeader()),
				mimeType_(std::move(mimeType)),
				ifModifiedSinceHeader_(std::move(ifModifiedSinceHeader)),
				data_(data),
				context_(context),
				next_(next),
				file_(),
				fileStream_() { }

		private:
			/** Get file path for execute */
			const SharedString& getPath() const {
				return supportCompress_ ? compressedPath_ : filePath_;
			}

			/** Parse "Range: bytes=from-to" or "Range: bytes=from-" */
			std::pair<std::size_t, std::size_t> parseRangeHeader() const {
				if (!startsWith(rangeHeader_, RangePrefix)) {
					return { 0, std::numeric_limits<std::size_t>::max() };
				}
				std::string_view rangeStr = rangeHeader_.view().substr(sizeof(RangePrefix) - 1);
				if (rangeStr.find_first_of(',') != rangeStr.npos) {
					// multiple range is unsupported for now
					return { 0, std::numeric_limits<std::size_t>::max() };
				}
				std::size_t pos = rangeStr.find_first_of('-');
				if (pos == rangeStr.npos) {
					return { 0, std::numeric_limits<std::size_t>::max() };
				}
				std::size_t from = 0;
				std::size_t to = 0;
				if (!loadIntFromDec(rangeStr.data(), pos, from)) {
					return { 0, std::numeric_limits<std::size_t>::max() };
				}
				std::string_view lastStr = rangeStr.substr(pos + 1);
				if (lastStr.empty()) {
					return { from, std::numeric_limits<std::size_t>::max() };
				}
				if (!loadIntFromDec(lastStr.data(), lastStr.size(), to) || from > to) {
					return { 0, std::numeric_limits<std::size_t>::max() };
				}
				return { from, to };
			}

		private:
			SharedString filePath_;
			SharedString compressedPath_;
			bool supportCompress_;
			SharedString rangeHeader_;
			std::pair<std::size_t, std::size_t> range_;
			SharedString mimeType_;
			SharedString ifModifiedSinceHeader_;
			HttpServerRequestStaticFileHandlerData& data_;
			HttpContext& context_;
			HttpServerRequestHandlerIterator next_;
			seastar::file file_;
			seastar::input_stream<char> fileStream_;
		};
	}

	/** Return content of request file */
	seastar::future<> HttpServerRequestStaticFileHandler::handle(
		HttpContext& context,
		HttpServerRequestHandlerIterator next) const {
		auto& request = context.getRequest();
		auto& path = request.getUri().getPath();
		// check url base
		// this check is not necessary when register static file handler with
		// routing handler, but it's cheap so keep it for safety
		if (CPV_UNLIKELY(!startsWith(path, data_->urlBase))) {
			return (*next)->handle(context, next + 1);
		}
		// validate relative path is starts with /
		std::string_view relPath = path.view().substr(data_->urlBase.size());
		if (CPV_UNLIKELY(relPath.empty() || relPath.front() != '/')) {
			return (*next)->handle(context, next + 1);
		}
		// sanitize path
		if (CPV_UNLIKELY(!isSafePath(relPath))) {
			return (*next)->handle(context, next + 1);
		}
		// generate file path (and detect is compression supported)
		thread_local static SharedStringBuilder pathBuilder;
		pathBuilder.clear();
		pathBuilder.append(data_->pathBase).append(relPath).append(CompressedFileSuffix);
		auto& headers = request.getHeaders();
		bool supportCompress = (
			headers.getAcceptEncoding().view().find(CompressEncoding) !=
			std::string_view::npos);
		SharedString rangeHeader = headers.getHeader(constants::Range);
		SharedString ifModifiedSinceHeader = headers.getHeader(constants::IfModifiedSince);
		// get file content from cache if appropriate
		auto& response = context.getResponse();
		SharedString mimeType = getMimeType(relPath);
		if (data_->fileCache.maxSize() > 0 && rangeHeader.empty()) {
			HttpServerRequestStaticFileHandlerData::FileCacheEntry* entry = nullptr;
			if (supportCompress) {
				entry = data_->fileCache.get(SharedString::fromStatic(pathBuilder.view()));
				if (entry != nullptr) {
					return entry->reply(response, std::move(mimeType),
						std::move(ifModifiedSinceHeader), data_->cacheControl);
				}
			}
			entry = data_->fileCache.get(SharedString::fromStatic(
				pathBuilder.view().substr(0,
					pathBuilder.size() - sizeof(CompressedFileSuffix) - 1)));
			if (entry != nullptr) {
				return entry->reply(response, std::move(mimeType),
					std::move(ifModifiedSinceHeader), data_->cacheControl);
			}
		}
		// get file content from disk
		SharedString compressedPath(pathBuilder.view());
		SharedString filePath = compressedPath.share(
			0, compressedPath.size() - sizeof(CompressedFileSuffix) - 1);
		return seastar::do_with(StaticFileReplier(
			std::move(filePath), std::move(compressedPath), supportCompress,
			std::move(rangeHeader), std::move(mimeType), std::move(ifModifiedSinceHeader),
			*data_, context, next),
			[] (StaticFileReplier& reply) {
			return reply.execute();
		});
	}

	/** Clear cached file contents */
	void HttpServerRequestStaticFileHandler::clearCache() {
		data_->fileCache.clear();
	}

	/** Constructor */
	HttpServerRequestStaticFileHandler::HttpServerRequestStaticFileHandler(
		SharedString&& urlBase,
		SharedString&& pathBase,
		SharedString&& cacheControl,
		std::size_t maxCacheFileEntities,
		std::size_t maxCacheFileSize) :
		data_(std::make_unique<HttpServerRequestStaticFileHandlerData>(
			std::move(urlBase),
			std::move(pathBase),
			std::move(cacheControl),
			maxCacheFileEntities,
			maxCacheFileSize)) { }

	/** Move constructor (for incomplete member type) */
	HttpServerRequestStaticFileHandler::HttpServerRequestStaticFileHandler(
		HttpServerRequestStaticFileHandler&&) = default;

	/** Move assign operator (for incomplete member type) */
	HttpServerRequestStaticFileHandler& HttpServerRequestStaticFileHandler::operator=(
		HttpServerRequestStaticFileHandler&&) = default;

	/** Destructor (for incomplete member type) */
	HttpServerRequestStaticFileHandler::~HttpServerRequestStaticFileHandler() = default;
}

