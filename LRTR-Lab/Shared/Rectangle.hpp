#pragma once

#include "Math/Vector.hpp"

namespace LRTR {

	template<typename T>
	struct Rectangle {
		Vector2<T> Min = Vector2<T>(0);
		Vector2<T> Max = Vector2<T>(0);

		Rectangle() = default;

		Rectangle(
			const Vector2<T>& min,
			const Vector2<T>& max) :
			Min(min), Max(max) {}

		Rectangle(
			const T& left, const T& top, const T& right, const T& bottom) :
			Min(Vector2<T>(left, top)),
			Max(Vector2<T>(right, bottom)) {}
	};

	using RectangleF = Rectangle<float>;
}