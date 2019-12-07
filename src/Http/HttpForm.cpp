#include <CPVFramework/Http/HttpForm.hpp>
#include <CPVFramework/Utility/ConstantStrings.hpp>
#include <CPVFramework/Utility/HttpUtils.hpp>

namespace cpv {
	/** Static empty vector for getMany */
	const HttpForm::ValuesType HttpForm::Empty;

	/** Parse url encoded form body */
	void HttpForm::parseUrlEncoded(const SharedString& body) {
		const char* mark = body.begin();
		const char* ptr = mark;
		const char* end = body.end();
		SharedString formKey;
		for (; ptr < end; ++ptr) {
			const char c = *ptr;
			if (c == '=') {
				// end of key, start of value
				formKey = urlDecode(body.share(
					{ mark, static_cast<std::size_t>(ptr - mark) }));
				mark = ptr + 1;
			} else if (c == '&') {
				// end of value
				add(std::move(formKey), urlDecode(body.share(
					{ mark, static_cast<std::size_t>(ptr - mark) })));
				mark = ptr + 1;
			}
		}
		if (ptr > mark || !formKey.empty()) {
			// end of value
			add(std::move(formKey), urlDecode(body.share(
				{ mark, static_cast<std::size_t>(ptr - mark) })));
		}
	}

	/** Apend url encoded form body to packet */
	void HttpForm::buildUrlEncoded(Packet& packet) {
		auto& fragments = packet.getOrConvertToMultiple();
		fragments.reserve_addition(formParameters_.size() * 4);
		bool isFirst = true;
		for (const auto& parameter : formParameters_) {
			for (const auto& value : parameter.second) {
				if (isFirst) {
					isFirst = false;
				} else {
					fragments.append(constants::Ampersand);
				}
				fragments.append(urlEncode(parameter.first.share()));
				fragments.append(constants::EqualsSign);
				fragments.append(urlEncode(value.share()));
			}
		}
	}

	/** Constructor */
	HttpForm::HttpForm() : formParameters_() { }
}

