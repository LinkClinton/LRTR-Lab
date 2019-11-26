#pragma once

#include "../../../Shared/Math/Math.hpp"
#include "../../../Shared/Triangle.hpp"

#include "../../Component.hpp"

namespace LRTR {

	class TrianglesMesh : public Component {
	public:
		TrianglesMesh() = default;
		
		explicit TrianglesMesh(
			const std::vector<TriangleF>& triangles);

		explicit TrianglesMesh(
			const std::vector<Vector3f>& vertices,
			const std::vector<unsigned>& indices);

		auto vertices() const noexcept -> const std::vector<Vector3f>&;

		auto indices() const noexcept -> const std::vector<unsigned>&;
		
		auto triangle(const size_t index) const -> TriangleF;

		auto size() const noexcept -> size_t;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		std::vector<Vector3f> mVertices;
		std::vector<unsigned> mIndices;

		size_t mCurrentTriangle = 0;
	};
	
}