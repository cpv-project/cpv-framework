#pragma once
#include "../Http/HttpForm.hpp"

namespace cpv {
	/**
	 * The class used to deserialize form to model.
	 * The model should contains a public function named `loadForm` and
	 * takes `const HttpForm&` as argument.
	 */
	template <class T, class = void /* for enable_if */>
	class FormDeserializer {
	public:
		/** Deserialize form to model */
		static void deserialize(T& model, const HttpForm& form) {
			model.loadForm(form);
		}
	};

	/** Convenient static function for FormDeserializer */
	template <class T>
	static inline void deserializeForm(T& model, const HttpForm& form) {
		return FormDeserializer<T>::deserialize(model, form);
	}
}

