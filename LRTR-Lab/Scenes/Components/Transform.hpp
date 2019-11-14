#pragma once

#include "../../Shared/Math/Math.hpp"
#include "../Component.hpp"

namespace LRTR {

	class Transform : public Component {
	public:
		Transform() = default;

		explicit Transform(const Matrix4x4f& transform);

		explicit Transform(const Matrix4x4f& transform, const Matrix4x4f& inverse);

		Transform& operator*(const Transform& right);

		template<typename T>
		Vector4<T> operator()(const Vector2<T>& vector) const;

		template<typename T>
		Vector4<T> operator()(const Vector3<T>& vector) const;

		template<typename T>
		Vector4<T> operator()(const Vector4<T>& vector) const;

		auto matrix() const noexcept->Matrix4x4f;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
		
		static auto inverse(const Transform& transform)->Transform;

		static auto translate(const Vector3f& delta)->Transform;

		static auto rotate(Real angle, const Vector3f& axis)->Transform;

		static auto scale(const Vector3f& value)->Transform;

		static auto lookAt(const Vector3f& eye, const Vector3f& at, const Vector3f& up)->Transform;

		static auto perspectiveFov(Real fovy, Real width, Real height, Real zNear, Real zFar)->Transform;
	protected:
		void onProperty() override;
	private:
		Matrix4x4f mTransform = Matrix4x4f(1);
		Matrix4x4f mInverse = Matrix4x4f(1);
	};

	template<typename T>
	inline Vector4<T> LRTR::Transform::operator()(const Vector2<T>& vector) const
	{
		return mTransform * Vector4<T>(vector.x, vector.y, 0, 1);
	}

	template<typename T>
	inline Vector4<T> LRTR::Transform::operator()(const Vector3<T>& vector) const
	{
		return mTransform * Vector4<T>(vector.x, vector.y, vector.z, 1);
	}

	template<typename T>
	inline Vector4<T> LRTR::Transform::operator()(const Vector4<T>& vector) const
	{
		return mTransform * Vector4<T>(vector.x, vector.y, vector.z, vector.w);
	}
}