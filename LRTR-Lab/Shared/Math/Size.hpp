#pragma once

namespace LRTR {

	template<typename T>
	struct Size2 {
		T Width;
		T Height;

		Size2() : Width(T()), Height(T()) {}

		Size2(const T& width, const T& height) :
			Width(width), Height(height) {}
	};
}