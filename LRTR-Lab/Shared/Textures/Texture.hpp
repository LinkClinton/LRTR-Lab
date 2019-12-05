#pragma once

#include "../../Core/Noncopyable.hpp"

namespace LRTR {

	class Texture : public Noncopyable {
	public:
		auto width() const noexcept -> size_t { return mWidth; }

		auto height() const noexcept -> size_t { return mHeight; }
	protected:
		Texture(const size_t width, const size_t height) :
			mWidth(width), mHeight(height) {}
	private:
		size_t mWidth;
		size_t mHeight;
	};
	
}