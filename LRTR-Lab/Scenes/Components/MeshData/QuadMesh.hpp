#pragma once

#include "TrianglesMesh.hpp"

namespace LRTR {

	class QuadMesh : public TrianglesMesh {
	public:
		explicit QuadMesh(const float width, const float height);

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		float mWidth;
		float mHeight;
	};
	
}