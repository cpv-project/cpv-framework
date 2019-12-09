#pragma once
#include "../Http/HttpForm.hpp"
#include "../Utility/ObjectTrait.hpp"

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
			using Trait = ObjectTrait<T>;
			Packet p;
			if constexpr (Trait::IsPointerLike) {
				if (Trait::get(model) == nullptr) {
					return p;
				}
			}
			HttpForm form;
			if constexpr (Trait::IsPointerLike) {
				Trait::get(model)->dumpForm(form);
			} else {
				Trait::get(model).dumpForm(form);
			}
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

