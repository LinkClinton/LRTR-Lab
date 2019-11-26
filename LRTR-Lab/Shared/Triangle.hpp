#pragma once

#include "Math/Vector.hpp"
#include <array>

namespace LRTR {


	template<typename T>
	struct Triangle {
		std::array<Vector3<T>, 3> Vertices;

		Triangle() = default;

		Triangle(const std::array<Vector3<T>, 3>& vertices) : Vertices(vertices) {}

		Triangle(
			const Vector3<T>& v0,
			const Vector3<T>& v1,
			const Vector3<T>& v2) :
			Vertices({ v0, v1, v2 }) {}
	};

	using TriangleF = Triangle<float>;
	
}