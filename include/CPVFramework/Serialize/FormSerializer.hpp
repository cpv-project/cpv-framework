#pragma once
#include "../Http/HttpForm.hpp"

namespace cpv {
	/**
	 * The class used to serialize model to url encoded form packet.
	 * The model should contains a public function named `dumpForm` and
	 * takes `HttpForm&` as argument.
	 */
	template <class T, class = void /* for enable_if */>
	class FormSerializer {
	public:
		/** Serialize model to form packet */
		static Packet serialize(const T& model) {
			HttpForm form;
			model.dumpForm(form);
			Packet p;
			form.buildUrlEncoded(p);
			return p;
		}
	};

	/** Convenient static function for FormSerializer */
	template <class T>
	static inline Packet serializeForm(const T& model) {
		return FormSerializer<T>::serialize(model);
	}
}

