#pragma once

namespace LRTR {

	template<typename T>
	struct Color {
		T Red, Green, Blue, Alpha;

		Color() : Red(0), Green(0), Blue(0), Alpha(1) {}

		Color(
			const T& red, const T& green, const T& blue, const T& alpha) :
			Red(red), Green(green), Blue(blue), Alpha(alpha) {}
	};

	using ColorF = Color<float>;
	
}