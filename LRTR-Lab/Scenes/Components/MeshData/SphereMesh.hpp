#pragma once

#include "TrianglesMesh.hpp"

namespace LRTR {

	class SphereMesh : public TrianglesMesh {
	public:
		explicit SphereMesh(const float radius) : SphereMesh(radius, 32, 32) {}

		explicit SphereMesh(const float radius, const int slice, const int stack);

		auto typeName() const noexcept -> std::string override;
		
		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		float mRadius;
	};
	
}
