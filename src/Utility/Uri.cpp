#include <CPVFramework/Utility/ConstantStrings.hpp>
#include <CPVFramework/Utility/HttpUtils.hpp>
#include <CPVFramework/Utility/Macros.hpp>
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

		inline std::string_view urlDecodeForUriParser(
			const char* begin, const char* end, Uri::UnderlyingBuffersType& underlyingBuffers) {
			auto result = urlDecode(std::string_view(begin, end - begin));
			if (result.second.size() > 0) {
				underlyingBuffers.emplace_back(std::move(result.second));
			}
			return result.first;
		}

		inline void urlEncodeThenAppendToPacket(std::string_view view, Packet& packet) {
			auto result = urlEncode(view);
			if (result.second.size() == 0) {
				packet.append(result.first);
			} else {
				packet.append(std::move(result.second));
			}
		}
	}
	
	/** Parse given uri */
	void Uri::parse(std::string_view uri) {
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
		std::string_view queryKey;
		for (; ptr < end; ++ptr) {
			const char c = *ptr;
			if (c == '/') {
				if (state == UriParserState::Protocol) {
					if (ptr != mark) {
						// path only without leading /, e.g. articles/today
						pathFragments_.emplace_back(
							urlDecodeForUriParser(mark, ptr, underlyingBuffers_));
					}
					// if ptr == mark, that mean first character is /, pathMark == mark
					mark = ptr + 1;
					state = UriParserState::Path;
				} else if (state == UriParserState::Path) {
					// end of following path segments
					pathFragments_.emplace_back(
						urlDecodeForUriParser(mark, ptr, underlyingBuffers_));
					mark = ptr + 1;
				} else if (state == UriParserState::Hostname) {
					// end of hostname, start of path
					hostname_ = urlDecodeForUriParser(mark, ptr, underlyingBuffers_);
					mark = ptr + 1;
					pathMark = ptr;
					state = UriParserState::Path;
				} else if (state == UriParserState::Port) {
					// end of port, start of path
					// port should be numeric, thus no url decode is required
					port_ = std::string_view(mark, ptr - mark);
					mark = ptr + 1;
					pathMark = ptr;
					state = UriParserState::Path;
				}
			} else if (c == '?') {
				if (state == UriParserState::Path) {
					// end of path, start of query parameters
					pathFragments_.emplace_back(
						urlDecodeForUriParser(mark, ptr, underlyingBuffers_));
					path_ = urlDecodeForUriParser(pathMark, ptr, underlyingBuffers_);
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
					queryKey = urlDecodeForUriParser(mark, ptr, underlyingBuffers_);
					mark = ptr + 1;
				}
			} else if (c == '&') {
				if (state == UriParserState::Query) {
					// end of value
					queryParameters_.insert_or_assign(queryKey,
						urlDecodeForUriParser(mark, ptr, underlyingBuffers_));
					mark = ptr + 1;
				}
			} else if (c == ':') {
				if (state == UriParserState::Protocol) {
					// end of protocol, start of hostname
					protocol_ = urlDecodeForUriParser(mark, ptr, underlyingBuffers_);
					ptr += 2;
					mark = ptr + 1;
					state = UriParserState::Hostname;
				} else if (state == UriParserState::Hostname) {
					// end of hostname, start of port
					hostname_ = urlDecodeForUriParser(mark, ptr, underlyingBuffers_);
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
					hostname_ = urlDecodeForUriParser(mark, ptr + 1, underlyingBuffers_);
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
				pathFragments_.emplace_back(
					urlDecodeForUriParser(mark, ptr, underlyingBuffers_));
			} else if (state == UriParserState::Hostname) {
				// ends with hostname
				hostname_ = urlDecodeForUriParser(mark, ptr, underlyingBuffers_);
			} else if (state == UriParserState::Port) {
				// ends with port
				port_ = std::string_view(mark, ptr - mark);
			} else if (state == UriParserState::Query) {
				// ends with query value
				queryParameters_.insert_or_assign(queryKey,
					urlDecodeForUriParser(mark, ptr, underlyingBuffers_));
			}
		}
		if (state == UriParserState::Path) {
			// ends with path, path may be "/" so don't put this inside if (ptr > mark)
			path_ = urlDecodeForUriParser(pathMark, ptr, underlyingBuffers_);
		}
	}
	
	/**
	 * Parse given uri
	 * Newly allocated contents for url decoder will append to underlyingBuffers.
	 */
	void Uri::parse(seastar::temporary_buffer<char>&& uri) {
		std::string_view view(uri.get(), uri.size());
		addUnderlyingBuffer(std::move(uri));
		parse(view);
	}
	
	/** Append uri string to packet */
	void Uri::build(Packet& packet) const {
		if (!protocol_.empty()) {
			urlEncodeThenAppendToPacket(protocol_, packet);
			packet.append(constants::ColonSlashSlash);
			urlEncodeThenAppendToPacket(hostname_, packet);
			if (!port_.empty()) {
				packet.append(constants::Colon).append(port_);
			}
		}
		if (!pathFragments_.empty()) {
			// perfer path segments if it's set
			// in case user parse an url, modify single fragment, then rebuild
			for (const auto& fragment : pathFragments_) {
				packet.append(constants::Slash);
				urlEncodeThenAppendToPacket(fragment, packet);
			}
		} else if (!path_.empty()) {
			urlEncodeThenAppendToPacket(path_, packet);
		}
		if (!queryParameters_.empty()) {
			packet.append(constants::QuestionMark);
			bool isFirst = true;
			for (const auto& parameter : queryParameters_) {
				if (isFirst) {
					isFirst = false;
				} else {
					packet.append(constants::Ampersand);
				}
				urlEncodeThenAppendToPacket(parameter.first, packet);
				packet.append(constants::EqualsSign);
				urlEncodeThenAppendToPacket(parameter.second, packet);
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
		underlyingBuffers_.clear();
	}
	
	/** Constructor */
	Uri::Uri() :
		protocol_(),
		hostname_(),
		port_(),
		path_(),
		pathFragments_(),
		queryParameters_(),
		underlyingBuffers_() { }
}

