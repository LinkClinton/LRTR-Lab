#pragma once

#include "TrianglesMesh.hpp"

namespace LRTR {

	class BoxMesh : public TrianglesMesh {
	public:
		explicit BoxMesh(const float width, const float height, const float depth);

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		float mWidth;
		float mHeight;
		float mDepth;
	};
	
}