#include <algorithm>
#include <unordered_map>
#include <utility>
#include <seastar/core/shared_ptr.hh>
#include <CPVFramework/Utility/Uri.hpp>
#include <CPVFramework/HttpServer/Handlers/HttpServerRequestRoutingHandler.hpp>

namespace cpv {
	/** Handler map associated with a single routing node */
	class HttpServerRequestRoutingHandlerMap {
	public:
		/** Get handler associated with given method, return nullptr for not exists */
		seastar::shared_ptr<HttpServerRequestHandlerBase> get(
			const SharedString& method) const {
			if (method == constants::GET) {
				return getHandler;
			} else if (method == constants::POST) {
				return postHandler;
			}
			auto it = handlers.find(method);
			if (it != handlers.end()) {
				return it->second;
			}
			return {};
		}

		/** Associate handler with given method */
		void set(SharedString&& method,
			const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler) {
			if (method == constants::GET) {
				getHandler = handler;
			} else if (method == constants::POST) {
				postHandler = handler;
			}
			handlers.emplace(std::move(method), handler);
		}

		/** Remove handler associated with given method */
		void remove(const SharedString& method) {
			if (method == constants::GET) {
				getHandler = nullptr;
			} else if (method == constants::POST) {
				postHandler = nullptr;
			}
			handlers.erase(method);
		}

		/** Constructor */
		HttpServerRequestRoutingHandlerMap() :
			getHandler(),
			postHandler(),
			handlers() { }

	public:
		seastar::shared_ptr<HttpServerRequestHandlerBase> getHandler;
		seastar::shared_ptr<HttpServerRequestHandlerBase> postHandler;
		std::unordered_map<SharedString,
			seastar::shared_ptr<HttpServerRequestHandlerBase>> handlers;
	};

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
					childIt = node->childs.emplace(pathFragment.share(),
						std::make_unique<HttpServerRequestRoutingNode>()).first;
				}
				node = childIt->second.get();
			}
			return node;
		}

		/** Constructor */
		HttpServerRequestRoutingNode() :
			map(),
			childs() { }

	public:
		// { method: handler, ... }
		HttpServerRequestRoutingHandlerMap map;
		// { pathFragment: node, ... }
		std::unordered_map<SharedString,
			std::unique_ptr<HttpServerRequestRoutingNode>> childs;
	};

	/** Members of HttpServerRequestRoutingHandler */
	class HttpServerRequestRoutingHandlerData {
	public:
		/** Constructor */
		HttpServerRequestRoutingHandlerData() :
			fullPathRoutingMap(),
			wildcardRoutingTree(std::make_unique<HttpServerRequestRoutingNode>()) { }

	public:
		// { fullPath: { method: handler, ... }, ... }
		std::unordered_map<SharedString,
			HttpServerRequestRoutingHandlerMap> fullPathRoutingMap;
		// node { { pathFragment: node, ... }, { method: handler, ... } }
		std::unique_ptr<HttpServerRequestRoutingNode> wildcardRoutingTree;
	};

	/** Associate handler with given method and path */
	void HttpServerRequestRoutingHandler::route(
		SharedString&& method, SharedString&& path,
		const seastar::shared_ptr<HttpServerRequestHandlerBase>& handler) {
		if (path.view().find_first_of('*') == std::string_view::npos) {
			// path not contains * at all
			data_->fullPathRoutingMap[std::move(path)].set(std::move(method), handler);
			return;
		}
		// parse path as uri
		Uri uri(path);
		auto& pathFragments = uri.getPathFragments();
		bool containsWildcard = std::count_if(
			pathFragments.begin(), pathFragments.end(),
			[] (auto& f) { return f == "*" || f == "**"; }) > 0;
		if (!containsWildcard) {
			// path contains * but not as a path fragment (e.g. /abc/*123/321* is a full path)
			data_->fullPathRoutingMap[std::move(path)].set(std::move(method), handler);
			return;
		}
		// add nodes to wildcard routing tree
		auto* node = data_->wildcardRoutingTree->findOrCreateForModify(pathFragments);
		node->map.set(std::move(method), handler);
	}

	/** Remove associated handler with given method and path */
	void HttpServerRequestRoutingHandler::removeRoute(
		const SharedString& method, const SharedString& path) {
		// erase from full path routing map
		auto it = data_->fullPathRoutingMap.find(path);
		if (it != data_->fullPathRoutingMap.end()) {
			it->second.remove(method);
		}
		// erase from wildcard routing tree
		Uri uri(path);
		auto* node = data_->wildcardRoutingTree->findForModify(uri.getPathFragments());
		if (node != nullptr) {
			node->map.remove(method);
		}
	}

	/** Get associated handler with given method and uri, return nullptr if not found */
	seastar::shared_ptr<HttpServerRequestHandlerBase>
	HttpServerRequestRoutingHandler::getRoute(
		const SharedString& method, const Uri& uri) const {
		// get handler from full path routing map
		auto it = data_->fullPathRoutingMap.find(uri.getPath());
		if (it != data_->fullPathRoutingMap.end()) {
			return it->second.get(method);
		}
		// get handler from wildcard routing tree
		auto* node = data_->wildcardRoutingTree->findForRoute(uri.getPathFragments());
		if (node != nullptr) {
			return node->map.get(method);
		}
		// not found
		return nullptr;
	}

	/** Get associated handler with given method and path, return nullptr if not found */
	seastar::shared_ptr<HttpServerRequestHandlerBase>
	HttpServerRequestRoutingHandler::getRoute(
		const SharedString& method, const SharedString& path) const {
		return getRoute(method, Uri(path));
	}

	/** Handle request depends on the routing table */
	seastar::future<> HttpServerRequestRoutingHandler::handle(
		HttpContext& context,
		HttpServerRequestHandlerIterator next) const {
		// notice here will not use getRoute, because:
		// - getRoute will parse uri if not found in full path routing map,
		//   but we should use request.getUri() so other code and reuse the parse result
		auto& request = context.getRequest();
		auto& method = request.getMethod();
		auto& path = request.getUrl();
		// optimize for fast path
		auto it = data_->fullPathRoutingMap.find(path);
		if (it != data_->fullPathRoutingMap.end()) {
			auto& map = it->second;
			if (method == constants::GET && map.getHandler != nullptr) {
				return map.getHandler->handle(context, next);
			} else if (method == constants::POST && map.postHandler != nullptr) {
				return map.postHandler->handle(context, next);
			}
			auto hit = map.handlers.find(method);
			if (hit != map.handlers.end() && hit->second != nullptr) {
				return hit->second->handle(context, next);
			}
		}
		// normal path (url constants query string or wildcard is needed)
		auto handler = getRoute(method, request.getUri());
		if (handler != nullptr) {
			return handler->handle(context, next);
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

