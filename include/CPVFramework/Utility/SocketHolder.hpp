#pragma once
#include <memory>
#include <seastar/net/api.hh>
#include "../Exceptions/LogicException.hpp"
#include "./Macros.hpp"

namespace cpv {
	/**
	 * Class use to delay the memory release phase of socket instance
	 * Since close a socket is an asynchronous operation and destruction is not,
	 * delete a socket without close it will cause use-after-free error,
	 * this class will help close the socket in background and the destructor can return immediately.
	 *
	 * Notice this class uses data_sink instead output_stream, because it's problematic,
	 * in many situations, output_stream will construct with batch_flushes,
	 * that will make buffer still in-use even after flush is called and the result future is resolved.
	 */
	class SocketHolder {
	public:
		/** Return whether the socket is connected */
		bool isConnected() const { return state_ != nullptr; }

		/** Get the socket */
		seastar::connected_socket& socket() {
			if (CPV_UNLIKELY(state_ == nullptr)) {
				throw LogicException(CPV_CODEINFO, "socket is not connected");
			}
			return state_->socket;
		}

		/** Get the input data source of the socket */
		seastar::input_stream<char>& in() {
			if (CPV_UNLIKELY(state_ == nullptr)) {
				throw LogicException(CPV_CODEINFO, "socket is not connected");
			}
			return state_->in;
		}

		/** Get the output data sink of the socket */
		seastar::data_sink& out() {
			if (CPV_UNLIKELY(state_ == nullptr)) {
				throw LogicException(CPV_CODEINFO, "socket is not connected");
			}
			return state_->out;
		}

		/** Close this socket */
		seastar::future<> close() {
			if (state_ == nullptr) {
				return seastar::make_ready_future<>();
			}
			return seastar::do_with(std::move(state_), [] (auto& state) {
				return state->in.close().then([&state] {
					return state->out.close();
				});
			});
		}

		/** Constructor */
		SocketHolder() : state_(nullptr) { }

		/** Constructor */
		explicit SocketHolder(seastar::connected_socket&& socket) :
			state_(std::make_unique<State>()) {
			state_->socket = std::move(socket);
			state_->in = state_->socket.input();
			state_->out = state_->socket.output().detach();
		}

		/** Move Constructor */
		SocketHolder(SocketHolder&& socket) noexcept :
			state_(std::move(socket.state_)) { }

		/** Move assignment */
		SocketHolder& operator=(SocketHolder&& socket) noexcept {
			if (CPV_LIKELY(&socket != this)) {
				(void)close();
				state_ = std::move(socket.state_);
			}
			return *this;
		}

		/** Disallow copy */
		SocketHolder(const SocketHolder&) = delete;
		SocketHolder& operator=(const SocketHolder&) = delete;

		/** Destructor */
		~SocketHolder() {
			// ignore exceptions
			(void)close();
		}

	private:
		struct State {
			seastar::connected_socket socket;
			seastar::input_stream<char> in;
			seastar::data_sink out;
		};
		std::unique_ptr<State> state_;
	};
}

