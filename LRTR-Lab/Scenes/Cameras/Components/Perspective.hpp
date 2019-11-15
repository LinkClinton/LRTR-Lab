#pragma once

#include "Projective.hpp"

namespace LRTR {

	class Perspective : public Projective {
	public:
		Perspective();
		
		explicit Perspective(
			const float fovy,
			const float width,
			const float height,
			const float zNear = 0.f,
			const float zFar = 1000.0f);

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		float mFovy;
		float mWidth;
		float mHeight;

		float mZNear;
		float mZFar;
	};
	
}