#pragma once

#include "../Math/Math.hpp"

#include "Texture.hpp"

namespace LRTR {

	template<typename T>
	class ConstantTexture : public Texture {
	public:
		explicit ConstantTexture(const T& value);

		auto value() const noexcept -> T;
	private:
		T mValue;
	};

	template <typename T>
	ConstantTexture<T>::ConstantTexture(const T& value) :
		Texture(1, 1), mValue(value)
	{
	}

	template <typename T>
	auto ConstantTexture<T>::value() const noexcept -> T
	{
		return mValue;
	}

	using ConstantTexture1F = ConstantTexture<Vector1f>;
	using ConstantTexture2F = ConstantTexture<Vector2f>;
	using ConstantTexture3F = ConstantTexture<Vector3f>;
	using ConstantTexture4F = ConstantTexture<Vector4f>;
}
