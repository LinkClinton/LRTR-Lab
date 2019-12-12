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

		explicit TransformWrap(
			const Vector3f& translation,
			const QuaternionF& rotation,
			const Vector3f& scale);

		void set(
			const Vector3f& translate,
			const Vector4f& rotate,
			const Vector3f& scale);

		void set(
			const Vector3f& translation,
			const QuaternionF& rotation,
			const Vector3f& scale);
		
		auto translation() const noexcept -> Vector3f;

		auto rotation() const noexcept -> QuaternionF;

		auto scale() const noexcept -> Vector3f;
		
		auto transform() const noexcept -> Transform;

		auto typeName() const noexcept->std::string override;
		
		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		Transform mTransform;

		Vector3f mTranslation = Vector3f();
		QuaternionF mRotation = QuaternionF();
		Vector3f mScale = Vector3f(1, 1, 1);
	};
	
}