#pragma once

#include "Texture.hpp"

#include <CodeRed/Interface/GpuResource/GpuTexture.hpp>

#include <memory>

namespace LRTR {

	class ImageTexture : public Texture {
	public:
		explicit ImageTexture(const std::shared_ptr<CodeRed::GpuTexture>& texture);

		auto value() const noexcept -> std::shared_ptr<CodeRed::GpuTexture>;
	private:
		std::shared_ptr<CodeRed::GpuTexture> mTexture;
	};

	inline ImageTexture::ImageTexture(const std::shared_ptr<CodeRed::GpuTexture>& texture) :
		Texture(texture->width(), texture->height()), mTexture(texture)
	{
	}

	inline auto ImageTexture::value() const noexcept -> std::shared_ptr<CodeRed::GpuTexture>
	{
		return mTexture;
	}

}
