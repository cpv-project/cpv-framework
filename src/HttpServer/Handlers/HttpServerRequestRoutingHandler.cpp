#include <algorithm>
#include <unordered_map>
#include <utility>
#include <seastar/core/shared_ptr.hh>
#include <CPVFramework/Utility/Uri.hpp>
#include <CPVFramework/Utility/HashUtils.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestRoutingHandler.hpp>

namespace cpv {
	/** Represents a single path fragment in the routing tree */
	class HttpServerRequestRoutingNode {
	public:
		/** Find child node for given path fragments for route, return nullptr if not found */
		const HttpServerRequestRoutingNode* findForRoute(
			const Uri::PathFragmentsType& pathFragments) const {
			const HttpServerRequestRoutingNode* node = this;
			for (const auto& pathFragment : pathFragments) {
				auto childIt = node->childs.find(pathFragment);
				if (childIt != node->childs.end()) {
					node = childIt->second.get();
					continue;
				}
				childIt = node->childs.find("**");
				if (childIt != node->childs.end()) {
					return childIt->second.get();
				}
				childIt = node->childs.find("*");
				if (childIt != node->childs.end()) {
					node = childIt->second.get();
				} else {
					return nullptr;
				}
			}
			return node;
		}

		/** Find child node for given path fragments for modify, return nullptr if not found */
		HttpServerRequestRoutingNode* findForModify(
			const Uri::PathFragmentsType& pathFragments) {
			HttpServerRequestRoutingNode* node = this;
			for (const auto& pathFragment : pathFragments) {
				auto childIt = node->childs.find(pathFragment);
				if (childIt == node->childs.end()) {
					return nullptr;
				}
				node = childIt->second.get();
			}
			return node;
		}

		/** Find or create child node for given path fragments for modify */
		HttpServerRequestRoutingNode* findOrCreateForModify(
			const Uri::PathFragmentsType& pathFragments) {
			HttpServerRequestRoutingNode* node = this;
			for (const auto& pathFragment : pathFragments) {
				auto childIt = node->childs.find(pathFragment);
				if (childIt == node->childs.end()) {
					childIt = node->childs.emplace(pathFragment,
						std::make_unique<HttpServerRequestRoutingNode>()).first;
				}
				node = childIt->second.get();
			}
			return node;
		}

		/** Constructor */
		HttpServerRequestRoutingNode() :
			handlers(),
			childs() { }

	public:
		// { method: handler, ... }
		std::unordered_map<std::string_view,
			seastar::shared_ptr<HttpServerRequestHandlerBase>> handlers;
		// { pathFragment: node, ... }
		std::unordered_map<std::string_view,
			std::unique_ptr<HttpServerRequestRoutingNode>> childs;
	};

	/** Members of HttpServerRequestRoutingHandler */
	class HttpServerRequestRoutingHandlerData {
	public:
		/** Get string view with same contents but the storage owned by this object */
		std::string_view remember(std::string_view view) {
			auto it = underlyingBuffers.find(view);
			if (it != underlyingBuffers.end()) {
				return std::string_view(it->second.get(), it->second.size());
			} else {
				seastar::temporary_buffer<char> buf(view.data(), view.size());
				std::string_view result(buf.get(), buf.size());
				underlyingBuffers.emplace(result, std::move(buf));
				return result;
			}
		}

		/** Constructor */
		HttpServerRequestRoutingHandlerData() :
			fullPathRoutingMap(),
			wildcardRoutingTree(std::make_unique<HttpServerRequestRoutingNode>()),
			underlyingBuffers() { }

	public:
		// { (method, fullPath): handler, ... }
		std::unordered_map<
			std::pair<std::string_view, std::string_view>,
			seastar::shared_ptr<HttpServerRequestHandlerBase>,
			hash<std::pair<std::string_view, std::string_view>>> fullPathRoutingMap;
		// node { { pathFragment: node, ... }, { method: handler, ... } }
		std::unique_ptr<HttpServerRequestRoutingNode> wildcardRoutingTree;
		// { view: buffer, ... }
		std::unordered_map<std::string_view, seastar::temporary_buffer<char>> underlyingBuffers;
	};

	/** Associate handler with given path */
	void HttpServerRequestRoutingHandler::route(
		std::string_view method, std::string_view path,
		const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler) {
		// remember method and path (hold the underlying buffers)
		std::string_view methodRemembered = data_->remember(method);
		std::string_view pathRemembered = data_->remember(path);
		if (pathRemembered.find_first_of('*') == pathRemembered.npos) {
			// path not contains * at all
			data_->fullPathRoutingMap.insert_or_assign(
				std::make_pair(methodRemembered, pathRemembered), handler);
			return;
		}
		// parse path as uri
		Uri uri(pathRemembered);
		auto& pathFragments = uri.getPathFragments();
		bool containsWildcard = std::count_if(
			pathFragments.begin(), pathFragments.end(),
			[] (auto& f) { return f == "*" || f == "**"; }) > 0;
		if (!containsWildcard) {
			// path contains * but not as a path fragment (e.g. /abc/*123/321* is a full path)
			data_->fullPathRoutingMap.insert_or_assign(
				std::make_pair(methodRemembered, pathRemembered), handler);
			return;
		}
		// add nodes to wildcard routing tree
		auto* node = data_->wildcardRoutingTree->findOrCreateForModify(pathFragments);
		node->handlers.insert_or_assign(methodRemembered, handler);
	}

	/** Remove associated handler with given path */
	void HttpServerRequestRoutingHandler::removeRoute(
		std::string_view method, std::string_view path) {
		// erase from full path routing map
		data_->fullPathRoutingMap.erase(std::make_pair(method, path));
		// erase from wildcard routing tree
		Uri uri(path);
		auto* node = data_->wildcardRoutingTree->findForModify(uri.getPathFragments());
		if (node != nullptr) {
			node->handlers.erase(method);
		}
	}

	/** Get associated handler with given path, return nullptr if not found */
	seastar::shared_ptr<HttpServerRequestHandlerBase>
	HttpServerRequestRoutingHandler::getRoute(
		std::string_view method, std::string_view path) const {
		// get handler from full path routing map
		bool containsQuery = path.find_first_of('?') != path.npos;
		if (!containsQuery) {
			auto it = data_->fullPathRoutingMap.find(std::make_pair(method, path));
			if (it != data_->fullPathRoutingMap.end()) {
				return it->second;
			}
		}
		Uri uri(path);
		if (containsQuery) {
			auto it = data_->fullPathRoutingMap.find(std::make_pair(method, uri.getPath()));
			if (it != data_->fullPathRoutingMap.end()) {
				return it->second;
			}
		}
		// get handler from wildcard routing tree
		auto* node = data_->wildcardRoutingTree->findForRoute(uri.getPathFragments());
		if (node != nullptr) {
			auto handlerIt = node->handlers.find(method);
			if (handlerIt != node->handlers.end()) {
				return handlerIt->second;
			}
		}
		// not found
		return nullptr;
	}

	/** Handle request depends on the routing table */
	seastar::future<> HttpServerRequestRoutingHandler::handle(
		HttpContext& context,
		const HttpServerRequestHandlerIterator& next) const {
		// notice here will not use getRoute, because:
		// - getRoute will make a copy of shared_ptr
		// - getRoute will parse uri if not found in full path routing map,
		//   but we should use request.getUri() so other code and reuse the parse result
		auto& request = context.getRequest();
		std::string_view method = request.getMethod();
		std::string_view path = request.getUrl();
		// use handler from full path routing map
		bool containsQuery = path.find_first_of('?') != path.npos;
		if (!containsQuery) {
			auto it = data_->fullPathRoutingMap.find(std::make_pair(method, path));
			if (it != data_->fullPathRoutingMap.end()) {
				return it->second->handle(context, next);
			}
		}
		Uri& uri = request.getUri();
		if (containsQuery) {
			auto it = data_->fullPathRoutingMap.find(std::make_pair(method, uri.getPath()));
			if (it != data_->fullPathRoutingMap.end()) {
				return it->second->handle(context, next);
			}
		}
		// get handler from wildcard routing tree
		auto* node = data_->wildcardRoutingTree->findForRoute(uri.getPathFragments());
		if (node != nullptr) {
			auto handlerIt = node->handlers.find(method);
			if (handlerIt != node->handlers.end()) {
				return handlerIt->second->handle(context, next);
			}
		}
		// not found, use next handler
		return (*next)->handle(context, next + 1);
	}

	/** Constructor */
	HttpServerRequestRoutingHandler::HttpServerRequestRoutingHandler() :
		data_(std::make_unique<HttpServerRequestRoutingHandlerData>()) { }

	/** Move constructor (for incomplete member type) */
	HttpServerRequestRoutingHandler::HttpServerRequestRoutingHandler(
		HttpServerRequestRoutingHandler&&) = default;

	/** Move assign operator (for incomplete member type) */
	HttpServerRequestRoutingHandler& HttpServerRequestRoutingHandler::operator=(
		HttpServerRequestRoutingHandler&&) = default;

	/** Destructor (for incomplete member type) */
	HttpServerRequestRoutingHandler::~HttpServerRequestRoutingHandler() = default;
}

