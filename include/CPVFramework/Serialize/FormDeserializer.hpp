#pragma once
#include "../Http/HttpForm.hpp"
#include "../Utility/ObjectTrait.hpp"

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
		static void deserialize(T& model, const SharedString& formBody) {
			using Trait = ObjectTrait<T>;
			if constexpr (Trait::IsPointerLike) {
				// if model is pointer type, ensure it's not nullptr
				if (Trait::get(model) == nullptr) {
					model = Trait::create();
				}
			}
			HttpForm form(formBody);
			if constexpr (Trait::IsPointerLike) {
				Trait::get(model)->loadForm(form);
			} else {
				Trait::get(model).loadForm(form);
			}
		}
	};

	/** Convenient static function for FormDeserializer */
	template <class T>
	static inline void deserializeForm(T& model, const SharedString& formBody) {
		return FormDeserializer<T>::deserialize(model, formBody);
	}
}

