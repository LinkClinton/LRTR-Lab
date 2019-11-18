#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace LRTR {

	using Real = float;

	using Vector1f = Vector1<float>;
	using Vector2f = Vector2<float>;
	using Vector3f = Vector3<float>;
	using Vector4f = Vector4<float>;
	using UInt32 = unsigned;

	using Matrix4x4f = Matrix4x4<float>;

	class MathUtility {
	public:
		template<typename T>
		static auto pi() -> T;
	};

	template <typename T>
	auto MathUtility::pi() -> T
	{
		return glm::pi<T>();
	}
}
