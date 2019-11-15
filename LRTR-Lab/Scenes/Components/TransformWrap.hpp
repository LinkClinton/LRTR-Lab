#pragma once

#include "../../Shared/Transform.hpp"
#include "../Component.hpp"

namespace LRTR {

	class TransformWrap : public Component {
	public:
		TransformWrap() = default;

		explicit TransformWrap(
			const Vector3f& translate,
			const Vector4f& rotate,
			const Vector3f& scale);

		void set(
			const Vector3f& translate,
			const Vector4f& rotate,
			const Vector3f& scale);
		
		auto translate() const noexcept -> Vector3f;

		auto rotate() const noexcept -> Vector4f;

		auto scale() const noexcept -> Vector3f;
		
		auto transform() const noexcept -> Transform;

		auto typeName() const noexcept->std::string override;
		
		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		Transform mTransform;

		Vector3f mTranslate = Vector3f(0);
		Vector4f mRotate = Vector4f(0, 0, 1, 0);
		Vector3f mScale = Vector3f(1);
	};
	
}