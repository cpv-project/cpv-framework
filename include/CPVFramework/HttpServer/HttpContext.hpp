#pragma once
#include <seastar/net/api.hh>
#include "../Container/Container.hpp"
#include "../Container/Container.hpp"
#include "../Container/ServiceStorage.hpp"
#include "../Http/HttpRequest.hpp"
#include "../Http/HttpResponse.hpp"

namespace cpv {
	/** The context type for handling single http request on http server */
	class HttpContext {
	public:
		/** Get the request received from http client */
		HttpRequest& getRequest() & { return request_; }
		const HttpRequest& getRequest() const& { return request_; }

		/** Get the response reply to http client */
		HttpResponse& getResponse() & { return response_; }
		const HttpResponse& getResponse() const& { return response_; }

		/** Get the socket address of http client */
		const seastar::socket_address& getClientAddress() const& { return clientAddress_; }

		/** Get service instance with service storage associated with this context */
		template <class TService>
		TService getService() const {
			return container_.get<TService>(serviceStorage_);
		}

		/** Get service instances with service storage associated with this context */
		template <class T, std::enable_if_t<ServiceTypeTrait<T>::IsCollection, int> = 0>
		std::size_t getManyServices(T& collection) const {
			return container_.getMany<T>(collection, serviceStorage_);
		}

		/** Update the request and the response in this context */
		void setRequestResponse(HttpRequest&& request, HttpResponse&& response) {
			request_ = std::move(request);
			response_ = std::move(response);
		}

		/** Set the socket address of http client */
		void setClientAddress(seastar::socket_address&& clientAddress) {
			clientAddress_ = std::move(clientAddress);
		}

		/** Update the container in this context */
		void setContainer(const Container& container) {
			container_ = container;
		}

		/** Clear the service storage, usually you should call it after setRequestResponse */
		void clearServiceStorage()	{
			serviceStorage_.clear();
		}

		/** Constructor */
		HttpContext() :
			request_(),
			response_(),
			clientAddress_(seastar::make_ipv4_address(0, 0)),
			container_(),
			serviceStorage_() { }

		/** Constructor for null context, should set members later */
		explicit HttpContext(nullptr_t) :
			request_(nullptr),
			response_(nullptr),
			clientAddress_(),
			container_(nullptr),
			serviceStorage_() { }

	private:
		HttpRequest request_;
		HttpResponse response_;
		seastar::socket_address clientAddress_;
		Container container_;
		mutable ServiceStorage serviceStorage_;
	};
}

