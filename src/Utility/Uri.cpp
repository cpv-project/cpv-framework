#include <CPVFramework/Utility/ConstantStrings.hpp>
#include <CPVFramework/Utility/HttpUtils.hpp>
#include <CPVFramework/Utility/Uri.hpp>

namespace cpv {
	namespace {
		enum class UriParserState {
			Protocol,
			Hostname,
			HostnameV6,
			Port,
			Path,
			Query
		};
	}
	
	/** Parse given uri */
	void Uri::parse(const SharedString& uri) {
		// examples:
		// http://www.example.com:8000/articles/today?sort=comments
		// Protocol => Hostname => Port => Path => Query
		// http://www.example.com/articles/today?sort=comments
		// Protocol => Hostname => Path => Query
		// http://[::1]:8000/articles/today?sort=comments
		// Protocol => Hostname => HostnameV6 => Port => Path => Query
		// http://[::1]/articles/today?sort=comments
		// Protocol => Hostname => HostnameV6 => Path => Query
		// /articles/today?sort=comments
		// Protocol => Path => Query
		// articles/today
		// Protocol => Path
		// ?sort=comments
		// Protocol => Query
		UriParserState state = UriParserState::Protocol;
		const char* mark = uri.begin();
		const char* pathMark = mark;
		const char* ptr = mark;
		const char* end = uri.end();
		SharedString queryKey;
		for (; ptr < end; ++ptr) {
			const char c = *ptr;
			if (c == '/') {
				if (state == UriParserState::Protocol) {
					if (ptr != mark) {
						// path only without leading /, e.g. articles/today
						pathFragments_.emplace_back(urlDecode(uri.share(
							{ mark, static_cast<std::size_t>(ptr - mark) })));
					}
					// if ptr == mark, that mean first character is /, pathMark == mark
					mark = ptr + 1;
					state = UriParserState::Path;
				} else if (state == UriParserState::Path) {
					// end of following path segments
					pathFragments_.emplace_back(urlDecode(uri.share(
						{ mark, static_cast<std::size_t>(ptr - mark) })));
					mark = ptr + 1;
				} else if (state == UriParserState::Hostname) {
					// end of hostname, start of path
					hostname_ = urlDecode(uri.share(
						{ mark, static_cast<std::size_t>(ptr - mark) }));
					mark = ptr + 1;
					pathMark = ptr;
					state = UriParserState::Path;
				} else if (state == UriParserState::Port) {
					// end of port, start of path
					// port should be numeric, thus no url decode is required
					port_ = uri.share({ mark, static_cast<std::size_t>(ptr - mark) });
					mark = ptr + 1;
					pathMark = ptr;
					state = UriParserState::Path;
				}
			} else if (c == '?') {
				if (state == UriParserState::Path) {
					// end of path, start of query parameters
					if (ptr > mark) {
						// /path/?key=value should not adding empty path fragment
						pathFragments_.emplace_back(urlDecode(uri.share(
							{ mark, static_cast<std::size_t>(ptr - mark) })));
					}
					path_ = urlDecode(uri.share(
						{ pathMark, static_cast<std::size_t>(ptr - pathMark) }));
					mark = ptr + 1;
					state = UriParserState::Query;
				} else if (state == UriParserState::Protocol) {
					// leading ?, e.g. ?sort=comments
					mark = ptr + 1;
					state = UriParserState::Query;
				}
			} else if (c == '=') {
				if (state == UriParserState::Query) {
					// end of key, start of value
					queryKey = urlDecode(uri.share(
						{ mark, static_cast<std::size_t>(ptr - mark) }));
					mark = ptr + 1;
				}
			} else if (c == '&') {
				if (state == UriParserState::Query) {
					// end of value
					queryParameters_.insert_or_assign(
						std::move(queryKey), urlDecode(uri.share(
							{ mark, static_cast<std::size_t>(ptr - mark) })));
					mark = ptr + 1;
				}
			} else if (c == ':') {
				if (state == UriParserState::Protocol) {
					// end of protocol, start of hostname
					protocol_ = urlDecode(uri.share(
						{ mark, static_cast<std::size_t>(ptr - mark) }));
					ptr += 2;
					mark = ptr + 1;
					state = UriParserState::Hostname;
				} else if (state == UriParserState::Hostname) {
					// end of hostname, start of port
					hostname_ = urlDecode(uri.share(
						{ mark, static_cast<std::size_t>(ptr - mark) }));
					mark = ptr + 1;
					state = UriParserState::Port;
				}
			} else if (c == '[') {
				if (state == UriParserState::Hostname) {
					// start of hostname v6, e.g. http://[::1]
					state = UriParserState::HostnameV6;
				}
			} else if (c == ']') {
				if (state == UriParserState::HostnameV6) {
					// end of hostname v6, start of port or path
					// hostname v6 will include [ and ]
					hostname_ = urlDecode(uri.share(
						{ mark, static_cast<std::size_t>(ptr + 1 - mark) }));
					mark = ptr + 1;
					if (mark < end && *mark == ':') {
						ptr += 1;
						mark += 1;
						state = UriParserState::Port;
					} else {
						pathMark = mark;
						state = UriParserState::Path;
					}
				}
			}
		}
		// parse final parts
		if (ptr > mark) {
			if (state == UriParserState::Path) {
				// ends with path
				pathFragments_.emplace_back(urlDecode(uri.share(
					{ mark, static_cast<std::size_t>(ptr - mark) })));
			} else if (state == UriParserState::Hostname) {
				// ends with hostname
				hostname_ = urlDecode(uri.share(
					{ mark, static_cast<std::size_t>(ptr - mark) }));
			} else if (state == UriParserState::Port) {
				// ends with port
				port_ = uri.share({ mark, static_cast<std::size_t>(ptr - mark) });
			} else if (state == UriParserState::Query) {
				// ends with query value
				queryParameters_.insert_or_assign(
					std::move(queryKey), urlDecode(uri.share(
						{ mark, static_cast<std::size_t>(ptr - mark) })));
			}
		}
		if (state == UriParserState::Path) {
			// ends with path, path may be "/" so don't put this inside if (ptr > mark)
			path_ = urlDecode(uri.share(
				{ pathMark, static_cast<std::size_t>(ptr - pathMark) }));
		}
	}
	
	/** Append uri string to packet */
	void Uri::build(Packet& packet) const {
		auto& fragments = packet.getOrConvertToMultiple();
		if (!protocol_.empty()) {
			fragments.append(urlEncode(protocol_.share()));
			fragments.append(constants::ColonSlashSlash);
			fragments.append(urlEncode(hostname_.share()));
			if (!port_.empty()) {
				fragments.append(constants::Colon);
				fragments.append(port_.share());
			}
		}
		if (!pathFragments_.empty()) {
			// perfer path segments if it's set
			// in case user parse an url, modify single fragment, then rebuild
			for (const auto& fragment : pathFragments_) {
				fragments.append(constants::Slash);
				fragments.append(urlEncode(fragment.share()));
			}
		} else if (!path_.empty()) {
			fragments.append(urlEncode(path_.share()));
		}
		if (!queryParameters_.empty()) {
			fragments.append(constants::QuestionMark);
			bool isFirst = true;
			for (const auto& parameter : queryParameters_) {
				if (isFirst) {
					isFirst = false;
				} else {
					fragments.append(constants::Ampersand);
				}
				fragments.append(urlEncode(parameter.first.share()));
				fragments.append(constants::EqualsSign);
				fragments.append(urlEncode(parameter.second.share()));
			}
		}
	}
	
	/** Reset the uri to empty state */
	void Uri::clear() {
		protocol_ = {};
		hostname_ = {};
		port_ = {};
		path_ = {};
		pathFragments_.clear();
		queryParameters_.clear();
	}
	
	/** Constructor */
	Uri::Uri() :
		protocol_(),
		hostname_(),
		port_(),
		path_(),
		pathFragments_(),
		queryParameters_() { }
}

