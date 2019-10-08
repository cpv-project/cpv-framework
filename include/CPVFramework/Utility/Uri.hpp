#pragma once
#include <seastar/core/temporary_buffer.hh>
#include <string_view>
#include "../Allocators/StackAllocator.hpp"
#include "./Packet.hpp"

namespace cpv {
	/**
	 * Class used to parse and build uri
	 *
	 * Example:
	 * http://www.mydomain.com:8000/articles/today?sort=comments
	 * ```
	 * protocol: "http"
	 * hostname: "www.mydomain.com"
	 * port: "8000"
	 * path: "/articles_today"
	 * pathFragments: [ "articles", "today" ]
	 * queryParameters: { "sort": "comments" }
	 * ```
	 * port will be empty string if omitted (will not determine by protocol automatically).
	 * protocol, hostname, port will be empty string if only path and query are given.
	 * pathFragments and queryParameters are optional, you can choose not to parse them.
	 *
	 * Notice:
	 * By default, ctor, parse and setters will not hold the owership of given string view,
	 * please use addUnderlyingBuffer if you want the uri instance to hold the owership.
	 * For performance reason, duplicated query parameter key is unsupported (e.g. ?a=1&a=2).
	 * Also for performance reason, uri parser will ignore all errors.
	 * Hash tag is not supported because it will never pass to server.
	 */
	class Uri {
	public:
		using PathFragmentsType = StackAllocatedVector<std::string_view, 6>;
		using QueryParametersType = StackAllocatedMap<std::string_view, std::string_view, 6>;
		using UnderlyingBuffersType = StackAllocatedVector<seastar::temporary_buffer<char>, 32>;
		
		// getters and setters
		std::string_view getProtocol() const { return protocol_; }
		std::string_view getHostname() const { return hostname_; }
		std::string_view getPort() const { return port_; }
		std::string_view getPath() const { return path_; }
		const PathFragmentsType& getPathFragments() const& { return pathFragments_; }
		PathFragmentsType& getPathFragments() & { return pathFragments_; }
		const QueryParametersType& getQueryParameters() const& { return queryParameters_; }
		QueryParametersType& getQueryParameters() & { return queryParameters_; }
		void setProtocol(std::string_view protocol) { protocol_ = protocol; }
		void setHostname(std::string_view hostname) { hostname_ = hostname; }
		void setPort(std::string_view port) { port_ = port; }
		void setPath(std::string_view path) { path_ = path; }
		
		/** Get path fragment for given index, return empty string if index out of range */
		std::string_view getPathFragment(std::size_t index) const {
			return index < pathFragments_.size() ?
				pathFragments_[index] : std::string_view();
		}
		
		/** Get query parameter for given key, return empty string if key not exists */
		std::string_view getQueryParameter(std::string_view key) const {
			auto it = queryParameters_.find(key);
			return it != queryParameters_.end() ? it->second : std::string_view();
		}
		
		/** Set query parameter associated with given key, will replace the exists one */
		void setQueryParameter(std::string_view key, std::string_view value) {
			queryParameters_.insert_or_assign(key, value);
		}
		
		/** Remove query parameter associated with given key */
		void removeQueryParameter(std::string_view key) {
			queryParameters_.erase(key);
		}
		
		/** Get underlying buffers */
		UnderlyingBuffersType& getUnderlyingBuffers() & { return underlyingBuffers_; }
		const UnderlyingBuffersType& getUnderlyingBuffers() const& { return underlyingBuffers_; }
		
		/** Add underlying buffer that owns the storage of string views */
		void addUnderlyingBuffer(seastar::temporary_buffer<char>&& buf) {
			underlyingBuffers_.emplace_back(std::move(buf));
		}
		
		/**
		 * Parse given uri
		 * Newly allocated contents for url decoder will append to underlyingBuffers.
		 * Notice it will not hold the owership of uri.
		 */
		void parse(std::string_view uri);
		
		/**
		 * Parse given uri
		 * Newly allocated contents for url decoder will append to underlyingBuffers.
		 */
		void parse(seastar::temporary_buffer<char>&& uri);
		
		/**
		 * Append uri string to packet
		 * If protocol is set, it will append "protocol://hostname{:port}/path{?query}",
		 * otherwise it will append "/path{?query}" only.
		 * Notice:
		 * packet will not hold the owership of contents, please ensure uri
		 * or the resource uri dependented is alive before packet sent.
		 */
		void build(Packet& packet) const;
		
		/**
		 * Reset the uri to empty state
		 * Please call this function before reuse this instance to parse another uri.
		 */
		void clear();
		
		/** Constructor */
		Uri();
		
		/** Constructor */
		explicit Uri(std::string_view uri) : Uri() { parse(uri); }
		
		/** Constructor */
		explicit Uri(seastar::temporary_buffer<char>&& uri) : Uri() { parse(std::move(uri)); }
		
	private:
		std::string_view protocol_;
		std::string_view hostname_;
		std::string_view port_;
		std::string_view path_;
		PathFragmentsType pathFragments_;
		QueryParametersType queryParameters_;
		UnderlyingBuffersType underlyingBuffers_;
	};
}

