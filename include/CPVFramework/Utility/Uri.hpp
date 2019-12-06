#pragma once
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
	 * For performance reason, duplicated query parameter key is unsupported (e.g. ?a=1&a=2).
	 * Also for performance reason, uri parser will ignore all errors.
	 * Hash tag is not supported because it will never pass to server.
	 */
	class Uri {
	public:
		using PathFragmentsType = StackAllocatedVector<SharedString, 6>;
		using QueryParametersType = StackAllocatedMap<SharedString, SharedString, 6>;
		
		// getters and setters
		const SharedString& getProtocol() const& { return protocol_; }
		const SharedString& getHostname() const& { return hostname_; }
		const SharedString& getPort() const& { return port_; }
		const SharedString& getPath() const& { return path_; }
		const PathFragmentsType& getPathFragments() const& { return pathFragments_; }
		PathFragmentsType& getPathFragments() & { return pathFragments_; }
		const QueryParametersType& getQueryParameters() const& { return queryParameters_; }
		QueryParametersType& getQueryParameters() & { return queryParameters_; }
		void setProtocol(SharedString&& protocol) { protocol_ = std::move(protocol); }
		void setHostname(SharedString&& hostname) { hostname_ = std::move(hostname); }
		void setPort(SharedString&& port) { port_ = std::move(port); }
		void setPath(SharedString&& path) { path_ = std::move(path); }
		
		/** Get path fragment for given index, return empty string if index out of range */
		SharedString getPathFragment(std::size_t index) const {
			return index < pathFragments_.size() ?
				pathFragments_[index].share() : SharedString();
		}
		
		/** Get query parameter for given key, return empty string if key not exists */
		SharedString getQueryParameter(const SharedString& key) const {
			auto it = queryParameters_.find(key);
			return it != queryParameters_.end() ? it->second.share() : SharedString();
		}
		
		/** Set query parameter associated with given key, will replace the exists one */
		void setQueryParameter(SharedString&& key, SharedString&& value) {
			queryParameters_.insert_or_assign(std::move(key), std::move(value));
		}
		
		/** Remove query parameter associated with given key */
		void removeQueryParameter(const SharedString& key) {
			queryParameters_.erase(key);
		}
		
		/** Parse given uri */
		void parse(const SharedString& uri);
		
		/**
		 * Append uri string to packet
		 * If protocol is set, it will append "protocol://hostname{:port}/path{?query}",
		 * otherwise it will append "/path{?query}" only.
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
		explicit Uri(const SharedString& uri) : Uri() { parse(uri); }
		
	private:
		SharedString protocol_;
		SharedString hostname_;
		SharedString port_;
		SharedString path_;
		PathFragmentsType pathFragments_;
		QueryParametersType queryParameters_;
	};
}

