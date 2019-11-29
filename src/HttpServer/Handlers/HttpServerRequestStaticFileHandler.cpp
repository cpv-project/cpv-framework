#include <array>
#include <seastar/core/reactor.hh>
#include <seastar/core/fstream.hh>
#include <seastar/core/future-util.hh>
#include <CPVFramework/Allocators/StackAllocator.hpp>
#include <CPVFramework/Exceptions/FileSystemException.hpp>
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
	namespace {
		/** Suffix of gzip compressed file */
		static const constexpr char CompressedFileSuffix[] = ".gz";
		/** Encoding string of gzip compressed content */
		static const constexpr char CompressEncoding[] = "gzip";
		/** Prefix for Range header, only bytes is supported */
		static const constexpr char RangePrefix[] = "bytes=";
		/** Prefix for Content-Range header, only bytes is supported */
		static const constexpr char ContentRangePrefix[] = "bytes ";
		/** Maximum size when reading file content to chunk */
		static const constexpr std::size_t ReadFileChunkSize = 4096;
		/** Header field for cache hit or miss */
		static const constexpr char XCacheHeader[] = "X-Cache";
		/** Header value for cache hit */
		static const constexpr char XCacheHitValue[] = "HIT";
	}

	/** Members of HttpServerRequestStaticFileHandler */
	class HttpServerRequestStaticFileHandlerData {
	public:
		/** File content and modified time */
		struct FileCacheEntry {
			SharedString content;
			SharedString lastModified;
			bool noCompressedVersion;

			/** Constructor */
			FileCacheEntry(
				SharedString&& contentVal,
				SharedString&& lastModifiedVal,
				bool noCompressedVersionVal) :
				content(std::move(contentVal)),
				lastModified(std::move(lastModifiedVal)),
				noCompressedVersion(noCompressedVersionVal) { }

			/** Reply to http response with 304 or 200 */
			seastar::future<> reply(
				HttpResponse& response,
				SharedString&& mimeType,
				SharedString&& ifModifiedSinceHeader,
				const SharedString& cacheControl,
				bool isCompressed) {
				// set Cache-Control if present
				auto& headers = response.getHeaders();
				if (!cacheControl.empty()) {
					headers.setCacheControl(cacheControl.share());
				}
				// set X-Cache to indicates cache hitted
				headers.setHeader(XCacheHeader, XCacheHitValue);
				// check if we can return 304 not modified
				if (ifModifiedSinceHeader == lastModified) {
					response.setStatusCode(constants::_304);
					response.setStatusMessage(constants::NotModified);
					headers.setContentType(std::move(mimeType));
					headers.setLastModified(std::move(ifModifiedSinceHeader));
					return seastar::make_ready_future<>();
				}
				// set Content-Encoding if compressed file is used
				if (isCompressed) {
					headers.setContentEncoding(CompressEncoding);
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
			pathBase.trim(trimString<false, true>(pathBase, "/\\"));
		}
	};

	/** Class for reply content of static file, allocate in once */
	class HttpServerRequestStaticFileReplier {
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
						noCompressedVersion_ = true;
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
						data_.fileCache.maxSize() > 0 && !range_.has_value()) {
						return fileStream_.read_exactly(fileSize).then(
							[this, lastModified=std::move(lastModified)] (auto buf) mutable {
							// store file content to cache
							data_.fileCache.set(
								getPath().share(), getPath().share(),
								{ buf.share(), lastModified.share(), noCompressedVersion_ });
							// set Last-Modified header
							auto& response = context_.getResponse();
							auto& headers = response.getHeaders();
							headers.setLastModified(std::move(lastModified));
							return extensions::reply(response,
								std::move(buf), std::move(mimeType_));
						});
					}
					// calculate actual range and add headers
					seastar::future<> seekFuture = seastar::make_ready_future<>();
					if (range_.has_value() &&
						range_->first < fileSize && range_->first <= range_->second) {
						// return partial content, notice to is inclusive: [form, to]
						// Content-Range example: bytes 200-1000/67589
						std::size_t from = range_->first;
						std::size_t to = range_->second;
						to = std::min(to, fileSize - 1);
						remainSize_ = to - from + 1;
						seekFuture = fileStream_.skip(from);
						response.setStatusCode(constants::_206);
						response.setStatusMessage(constants::PartialContent);
						headers.setHeader(constants::ContentRange,
							SharedStringBuilder(32).append(ContentRangePrefix)
								.append(from).append(constants::Hyphen)
								.append(to).append(constants::Slash)
								.append(fileSize).build());
					} else {
						// return full content
						remainSize_ = fileSize;
						response.setStatusCode(constants::_200);
						response.setStatusMessage(constants::OK);
						headers.setLastModified(std::move(lastModified));
					}
					headers.setContentType(std::move(mimeType_));
					headers.setContentLength(SharedString::fromInt(remainSize_));
					return seekFuture.then([this] {
						// read and reply file content by chunks
						return seastar::repeat([this] {
							std::size_t readSize = std::min(remainSize_, ReadFileChunkSize);
							return fileStream_.read_up_to(readSize).then([this] (auto buf) {
								if (buf.size() == 0) {
									return seastar::make_exception_future<>(FileSystemException(
										CPV_CODEINFO, "remain size > 0 but eof occurs"));
								}
								SharedString str(std::move(buf));
								remainSize_ -= str.size();
								return extensions::writeAll(
									context_.getResponse().getBodyStream(), std::move(str));
							}).then([this] {
								return remainSize_ == 0 ?
									seastar::stop_iteration::yes :
									seastar::stop_iteration::no;
							});
						});
					});
				});
			});
		}

		/** Constructor */
		HttpServerRequestStaticFileReplier(
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
			noCompressedVersion_(false),
			rangeHeader_(std::move(rangeHeader)),
			range_(parseRangeHeader()),
			mimeType_(std::move(mimeType)),
			ifModifiedSinceHeader_(std::move(ifModifiedSinceHeader)),
			data_(data),
			context_(context),
			next_(next),
			file_(),
			fileStream_(),
			remainSize_(0) { }

	private:
		/** Get file path for execute */
		const SharedString& getPath() const {
			return supportCompress_ ? compressedPath_ : filePath_;
		}

		/** Parse "Range: bytes=from-to" or "Range: bytes=from-" */
		std::optional<std::pair<std::size_t, std::size_t>> parseRangeHeader() const {
			if (!startsWith(rangeHeader_, RangePrefix)) {
				return std::nullopt;
			}
			std::string_view rangeStr = rangeHeader_.view().substr(sizeof(RangePrefix) - 1);
			if (rangeStr.find_first_of(',') != rangeStr.npos) {
				// multiple range is unsupported for now
				return std::nullopt;
			}
			std::size_t pos = rangeStr.find_first_of('-');
			if (pos == rangeStr.npos) {
				return std::nullopt;
			}
			std::size_t from = 0;
			std::size_t to = 0;
			if (!loadIntFromDec(rangeStr.data(), pos, from)) {
				return std::nullopt;
			}
			std::string_view lastStr = rangeStr.substr(pos + 1);
			if (lastStr.empty()) {
				return std::make_pair(from, std::numeric_limits<std::size_t>::max());
			}
			if (!loadIntFromDec(lastStr.data(), lastStr.size(), to)) {
				return std::nullopt;
			}
			return std::make_pair(from, to);
		}

	private:
		SharedString filePath_;
		SharedString compressedPath_;
		bool supportCompress_;
		bool noCompressedVersion_;
		SharedString rangeHeader_;
		std::optional<std::pair<std::size_t, std::size_t>> range_;
		SharedString mimeType_;
		SharedString ifModifiedSinceHeader_;
		HttpServerRequestStaticFileHandlerData& data_;
		HttpContext& context_;
		HttpServerRequestHandlerIterator next_;
		seastar::file file_;
		seastar::input_stream<char> fileStream_;
		std::size_t remainSize_;
	};

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
						std::move(ifModifiedSinceHeader), data_->cacheControl, true);
				}
			}
			entry = data_->fileCache.get(SharedString::fromStatic(
				pathBuilder.view().substr(0,
					pathBuilder.size() - sizeof(CompressedFileSuffix) + 1)));
			if (entry != nullptr && (!supportCompress || entry->noCompressedVersion)) {
				// client not support compress or ensure there no compressed version
				return entry->reply(response, std::move(mimeType),
					std::move(ifModifiedSinceHeader), data_->cacheControl, false);
			}
		}
		// disable compress when range header is presented
		if (!rangeHeader.empty()) {
			supportCompress = false;
		}
		// get file content from disk
		SharedString compressedPath(pathBuilder.view());
		SharedString filePath = compressedPath.share(
			0, compressedPath.size() - sizeof(CompressedFileSuffix) + 1);
		return seastar::do_with(std::make_unique<HttpServerRequestStaticFileReplier>(
			std::move(filePath), std::move(compressedPath), supportCompress,
			std::move(rangeHeader), std::move(mimeType), std::move(ifModifiedSinceHeader),
			*data_, context, next),
			[] (auto& reply) {
			return reply->execute();
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

