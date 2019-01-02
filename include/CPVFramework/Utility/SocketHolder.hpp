#pragma once
#include <memory>
#include <seastar/net/api.hh>
#include "../Exceptions/LogicException.hpp"

namespace cpv {
	/**
	 * Class use to delay the memory release phase of socket instance
	 * Since close a socket is an asynchronous operation and destruction is not,
	 * delete a socket without close it and it's opened stream will cause use-after-free error,
	 * this class will help close the socket in background and the destructor can return immediately.
	 */
	class SocketHolder {
	public:
		/** Return whether the socket is connected */
		bool isConnected() const { return state_ != nullptr; }

		/** Get the socket */
		seastar::connected_socket& socket() {
			if (state_ == nullptr) {
				throw LogicException(CPV_CODEINFO, "socket is not connected");
			}
			return state_->socket;
		}

		/** Get the input stream of the socket */
		seastar::input_stream<char>& in() {
			if (state_ == nullptr) {
				throw LogicException(CPV_CODEINFO, "socket is not connected");
			}
			return state_->in;
		}

		/** Get the output stream of the socket */
		seastar::output_stream<char>& out() {
			if (state_ == nullptr) {
				throw LogicException(CPV_CODEINFO, "socket is not connected");
			}
			return state_->out;
		}

		/** Constructor */
		SocketHolder() : state_(nullptr) { }

		/** Constructor */
		explicit SocketHolder(seastar::connected_socket&& socket) :
			state_(std::make_unique<State>()) {
			state_->socket = std::move(socket);
			state_->in = state_->socket.input();
			state_->out = state_->socket.output();
		}

		/** Move Constructor */
		SocketHolder(SocketHolder&& socket) noexcept :
			state_(std::move(socket.state_)) { }

		/** Move assignment */
		SocketHolder& operator=(SocketHolder&& socket) noexcept {
			if (&socket != this) {
				close();
				state_ = std::move(socket.state_);
			}
			return *this;
		}

		/** Disallow copy */
		SocketHolder(const SocketHolder&) = delete;
		SocketHolder& operator=(const SocketHolder&) = delete;

		/** Destructor */
		~SocketHolder() {
			close();
		}

	private:
		/** Close this socket */
		void close() {
			if (state_ == nullptr) {
				return;
			}
			seastar::do_with(std::move(state_), [] (auto& state) {
				return state->in.close().then([&state] {
					return state->out.close();
				});
			});
		}

	private:
		struct State {
			seastar::connected_socket socket;
			seastar::input_stream<char> in;
			seastar::output_stream<char> out;
		};
		std::unique_ptr<State> state_;
	};
}

