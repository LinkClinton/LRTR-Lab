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

		template<typename T>
		static auto cross(const T& v0, const T& v1) -> T;

		template<typename T>
		static auto normalize(const T& v) -> T;

		template<typename T>
		static auto dot(const T& v0, const T& v1) -> float;

		template<typename T>
		static auto acos(const T& v) -> T;
	};

	template <typename T>
	auto MathUtility::pi() -> T
	{
		return glm::pi<T>();
	}

	template <typename T>
	auto MathUtility::cross(const T& v0, const T& v1) -> T
	{
		return glm::cross(v0, v1);
	}

	template <typename T>
	auto MathUtility::normalize(const T& v) -> T
	{
		return glm::normalize(v);
	}

	template <typename T>
	auto MathUtility::dot(const T& v0, const T& v1) -> float
	{
		return glm::dot(v0, v1);
	}

	template <typename T>
	auto MathUtility::acos(const T& v) -> T
	{
		return glm::acos(v);
	}
}
