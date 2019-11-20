#include <array>
#include <fstream>
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
			seastar::temporary_buffer<char> content;
			seastar::temporary_buffer<char> lastModified;

			/** Constructor */
			FileCacheEntry(
				seastar::temporary_buffer<char>&& contentVal,
				seastar::temporary_buffer<char>&& lastModifiedVal) :
				content(std::move(contentVal)),
				lastModified(std::move(lastModifiedVal)) { }

			/** Reply to http response with 304 or 200 */
			seastar::future<> reply(
				HttpResponse& response,
				std::string_view mimeType,
				std::string_view cacheControl,
				std::string_view ifModifiedSinceHeader) {
				// set Cache-Control if present
				auto& headers = response.getHeaders();
				if (!cacheControl.empty()) {
					headers.setCacheControl(cacheControl);
				}
				// check if we can return 304 not modified
				if (ifModifiedSinceHeader ==
					std::string_view(lastModified.get(), lastModified.size())) {
					response.setStatusCode(constants::_304);
					response.setStatusMessage(constants::NotModified);
					headers.setContentType(mimeType);
					headers.setLastModified(ifModifiedSinceHeader);
					return seastar::make_ready_future<>();
				}
				// return cached file content
				headers.setLastModified(response.addUnderlyingBuffer(lastModified.share()));
				return extensions::reply(response, content.share(), mimeType);
			}
		};

		std::string urlBase;
		std::string pathBase;
		std::string cacheControl;
		std::size_t maxCacheFileSize;
		LRUCache<std::string, FileCacheEntry> fileCache;

		/** Constructor */
		HttpServerRequestStaticFileHandlerData(
			std::string_view urlBaseVal,
			std::string_view pathBaseVal,
			std::string_view cacheControlVal,
			std::size_t maxCacheFileEntitiesVal,
			std::size_t maxCacheFileSizeVal) :
			urlBase(trimString<false, true>(urlBaseVal, '/')),
			pathBase(trimString<false, true>(pathBaseVal, '/')),
			cacheControl(cacheControlVal),
			maxCacheFileSize(maxCacheFileSizeVal),
			fileCache(maxCacheFileEntitiesVal) { }
	};

	namespace {
		/** Suffix of gzip compressed file */
		static const constexpr std::string_view CompressedFileSuffix = ".gz";
		/** Encoding string of gzip compressed content */
		static const constexpr std::string_view CompressEncoding = "gzip";
		/** Prefix for Range header, only bytes is supported */
		static const constexpr std::string_view RangePrefix = "bytes=";

		/** Class for reply content of static file, allocate in once */
		class StaticFileReplier {
		public:
			/** Initialize after memory location is fixed */
			void init() {
				compressedPath_ = filePathStorage_;
				filePath_ = std::string_view(
					filePathStorage_.data(),
					filePathStorage_.size() - CompressedFileSuffix.size());
				if (startsWith(rangeHeader_, RangePrefix)) {
					// parse range header, multiple range is not supported for now
					std::string_view rangeStr = rangeHeader_.substr(RangePrefix.size());
					std::size_t pos = rangeStr.find_first_of('-');
					if (pos != rangeStr.npos) {
						loadIntFromDec(rangeStr.data(), pos, range_.first);
						std::string_view lastStr = rangeStr.substr(pos + 1);
						if (!lastStr.empty()) {
							loadIntFromDec(lastStr.data(), lastStr.size(), range_.second);
						}
					}
				}
			}

			/** Execute reply operation */
			seastar::future<> execute() {
				std::string_view path = supportCompress_ ? compressedPath_ : filePath_;
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
					std::string_view path = supportCompress_ ? compressedPath_ : filePath_;
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
							headers.setCacheControl(data_.cacheControl);
						}
						// check if we can return 304 not modified
						std::string_view lastModifiedStr = formatTimeForHttpHeader(st.st_mtime);
						if (ifModifiedSinceHeader_ == lastModifiedStr) {
							response.setStatusCode(constants::_304);
							response.setStatusMessage(constants::NotModified);
							headers.setContentType(mimeType_);
							headers.setLastModified(ifModifiedSinceHeader_);
							return seastar::make_ready_future<>();
						}
						// allocate temporary buffer for Last-Modified header
						seastar::temporary_buffer<char> lastModified(
							lastModifiedStr.data(), lastModifiedStr.size());
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
									supportCompress_ ? compressedPath_ : filePath_,
									HttpServerRequestStaticFileHandlerData::FileCacheEntry(
										buf.share(), lastModified.share()));
								// set Last-Modified header
								auto& response = context_.getResponse();
								auto& headers = response.getHeaders();
								headers.setLastModified(
									response.addUnderlyingBuffer(std::move(lastModified)));
								return extensions::reply(response, std::move(buf), mimeType_);
							});
						}
						// TODO
						return seastar::make_ready_future<>();
					});
				});
			}

			/** Constructor */
			StaticFileReplier(
				const std::string& filePathStorage,
				bool supportCompress,
				std::string_view rangeHeader,
				std::string_view mimeType,
				std::string_view ifModifiedSinceHeader,
				HttpServerRequestStaticFileHandlerData& data,
				HttpContext& context,
				HttpServerRequestHandlerIterator next) :
				filePathStorage_(filePathStorage),
				filePath_(), // assign after allocated to avoid sso invalidate
				compressedPath_(), // same as above
				supportCompress_(supportCompress),
				rangeHeader_(rangeHeader),
				range_(0, std::numeric_limits<std::size_t>::max()),
				mimeType_(mimeType),
				ifModifiedSinceHeader_(ifModifiedSinceHeader),
				data_(data),
				context_(context),
				next_(next),
				file_(),
				fileStream_() { }

		private:
			std::string filePathStorage_;
			std::string_view filePath_;
			std::string_view compressedPath_;
			bool supportCompress_;
			std::string_view rangeHeader_;
			std::pair<std::size_t, std::size_t> range_;
			std::string_view mimeType_;
			std::string_view ifModifiedSinceHeader_;
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
		const HttpServerRequestHandlerIterator& next) const {
		auto& request = context.getRequest();
		std::string_view path = request.getUri().getPath();
		// check url base
		// this check is not necessary when register static file handler with
		// routing handler, but it's cheap so keep it for safety
		if (CPV_UNLIKELY(!startsWith(path, data_->urlBase))) {
			return (*next)->handle(context, next + 1);
		}
		// validate relative path is starts with /
		std::string_view relPath = path.substr(data_->urlBase.size());
		if (CPV_UNLIKELY(relPath.empty() || relPath.front() != '/')) {
			return (*next)->handle(context, next + 1);
		}
		// sanitize path
		if (CPV_UNLIKELY(!isSafePath(relPath))) {
			return (*next)->handle(context, next + 1);
		}
		// generate file path (and detect is compression supported)
		thread_local static std::string tmpStorage;
		tmpStorage.clear();
		tmpStorage.append(data_->pathBase).append(relPath).append(CompressedFileSuffix);
		auto& headers = request.getHeaders();
		bool supportCompress = (
			headers.getAcceptEncoding().find(CompressEncoding) != std::string_view::npos);
		std::string_view rangeHeader = headers.getHeader(constants::Range);
		std::string_view ifModifiedSinceHeader = (
			headers.getHeader(constants::IfModifiedSince));
		// get file content from cache if appropriate
		auto& response = context.getResponse();
		std::string_view mimeType = getMimeType(relPath);
		if (data_->fileCache.maxSize() > 0 && rangeHeader.empty()) {
			HttpServerRequestStaticFileHandlerData::FileCacheEntry* entry = nullptr;
			if (supportCompress) {
				entry = data_->fileCache.get(tmpStorage);
				if (entry != nullptr) {
					return entry->reply(response, mimeType,
						data_->cacheControl, ifModifiedSinceHeader);
				}
			}
			entry = data_->fileCache.get(std::string_view(
				tmpStorage.data(), tmpStorage.size() - CompressedFileSuffix.size()));
			if (entry != nullptr) {
				return entry->reply(response, mimeType,
					data_->cacheControl, ifModifiedSinceHeader);
			}
		}
		// get file content from disk
		return seastar::do_with(StaticFileReplier(
			tmpStorage, supportCompress, rangeHeader, mimeType,
			ifModifiedSinceHeader, *data_, context, next),
			[] (StaticFileReplier& reply) {
			reply.init();
			return reply.execute();
		});
	}

	/** Clear cached file contents */
	void HttpServerRequestStaticFileHandler::clearCache() {
		data_->fileCache.clear();
	}

	/** Constructor */
	HttpServerRequestStaticFileHandler::HttpServerRequestStaticFileHandler(
		std::string_view urlBase,
		std::string_view pathBase,
		std::string_view cacheControl,
		std::size_t maxCacheFileEntities,
		std::size_t maxCacheFileSize) :
		data_(std::make_unique<HttpServerRequestStaticFileHandlerData>(
			urlBase, pathBase, cacheControl, maxCacheFileEntities, maxCacheFileSize)) { }

	/** Move constructor (for incomplete member type) */
	HttpServerRequestStaticFileHandler::HttpServerRequestStaticFileHandler(
		HttpServerRequestStaticFileHandler&&) = default;

	/** Move assign operator (for incomplete member type) */
	HttpServerRequestStaticFileHandler& HttpServerRequestStaticFileHandler::operator=(
		HttpServerRequestStaticFileHandler&&) = default;

	/** Destructor (for incomplete member type) */
	HttpServerRequestStaticFileHandler::~HttpServerRequestStaticFileHandler() = default;
}

