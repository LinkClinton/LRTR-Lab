#pragma once

#include "Quaternion.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Radius.hpp"
#include "Size.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace LRTR {

	using Real = float;

	using QuaternionF = Quaternion<float>;
	
	using Vector1f = Vector1<float>;
	using Vector2f = Vector2<float>;
	using Vector3f = Vector3<float>;
	using Vector4f = Vector4<float>;

	using Radius1f = Radius1<float>;
	using Radius2f = Radius2<float>;
	using Radius3f = Radius3<float>;

	using Size2f = Size2<float>;
	using Size2i = Size2<int>;
	using Size2u = Size2<unsigned>;
	
	using UInt32 = unsigned;

	using Matrix4x4f = Matrix4x4<float>;

	class MathUtility {
	public:
		template<typename T>
		static auto pi() -> T;

		template<typename T>
		static auto two_pi() ->T ;

		template<typename T>
		static auto cross(const T& v0, const T& v1) -> T;

		template<typename T>
		static auto normalize(const T& v) -> T;

		template<typename T>
		static auto dot(const T& v0, const T& v1) -> float;

		template<typename T>
		static auto acos(const T& v) -> T;

		template<typename T>
		static auto sin(const T& v) -> T;

		template<typename T>
		static auto cos(const T& v) -> T;
		
		template<typename T>
		static auto clamp(const T& value, const T& min, const T& max) -> T;

		template<typename T>
		static auto max(const T& v0, const T& v1) -> T;

		template<typename T>
		static auto min(const T& v0, const T& v1) -> T;
		
		template<typename T>
		static auto rotate(const Vector3<T>& from, const Vector3<T>& to) -> Vector4<T>;

		template<typename T>
		static void decompose(const Matrix4x4<T>& matrix,
			Vector3<T>& translate, Quaternion<T>& quaternion, Vector3<T>& scale);
	};

	template <typename T>
	auto MathUtility::pi() -> T
	{
		return glm::pi<T>();
	}

	template <typename T>
	auto MathUtility::two_pi() -> T
	{
		return glm::two_pi<T>();
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

	template <typename T>
	auto MathUtility::sin(const T& v) -> T
	{
		return glm::sin(v);
	}

	template <typename T>
	auto MathUtility::cos(const T& v) -> T
	{
		return glm::cos(v);
	}

	template <typename T>
	auto MathUtility::clamp(const T& value, const T& min, const T& max) -> T
	{
		return glm::clamp(value, min, max);
	}

	template <typename T>
	auto MathUtility::max(const T& v0, const T& v1) -> T
	{
		return glm::max(v0, v1);
	}

	template <typename T>
	auto MathUtility::min(const T& v0, const T& v1) -> T
	{
		return glm::min(v0, v1);
	}

	template <typename T>
	auto MathUtility::rotate(const Vector3<T>& from, const Vector3<T>& to) -> Vector4<T>
	{
		const auto normalFrom = normalize(from);
		const auto normalTo = normalize(to);
		
		const auto rAngle = acos(MathUtility::dot(normalFrom, normalTo));
		const auto rAxis = cross(normalFrom, normalTo);

		return rAxis == Vector3<T>(0) ? Vector4<T>(0, 0, 0, 0) : Vector4<T>(rAxis, rAngle);
	}

	template <typename T>
	void MathUtility::decompose(
		const Matrix4x4<T>& matrix, 
		Vector3<T>& translate, Quaternion<T>& quaternion, Vector3<T>& scale)
	{
		Vector3<T> skew;
		Vector4<T> perspective;
		
		glm::decompose(matrix, scale, quaternion, translate, skew, perspective);
	}
}
