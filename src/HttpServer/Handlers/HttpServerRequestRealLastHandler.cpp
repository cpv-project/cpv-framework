#include <CPVFramework/Exceptions/LogicException.hpp>
#include "./HttpServerRequestRealLastHandler.hpp"

namespace cpv {
	/** Return exception future */
	seastar::future<> HttpServerRequestRealLastHandler::handle(
		cpv::HttpContext&,
		const cpv::HttpServerRequestHandlerIterator&) const {
		return seastar::make_exception_future<>(LogicException(CPV_CODEINFO,
			"The last handler in handler list should not invoke next handler"));
	}
}

