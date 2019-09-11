#pragma once
#include "../Utility/Uri.hpp"

namespace cpv {
	/**
	 * Uri for http request
	 * Notice: this collection may contains string_view that it's storage is hold in HttpRequest
	 */
	class HttpRequestUri : public Uri {
	private:
		using Uri::Uri;
		using Uri::operator=;
		friend class HttpRequestData;
	};
}

