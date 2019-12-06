#pragma once

#include "../../../Shared/Triangle.hpp"

#include "MeshData.hpp"

namespace LRTR {

	class TrianglesMesh : public MeshData {
	public:
		TrianglesMesh() = default;
		
		explicit TrianglesMesh(
			const std::vector<TriangleF>& triangles);

		explicit TrianglesMesh(
			const std::vector<Vector3f>& vertices,
			const std::vector<unsigned>& indices);

		explicit TrianglesMesh(
			const std::vector<Vector3f>& texCoords,
			const std::vector<Vector3f>& vertices,
			const std::vector<Vector3f>& tangents,
			const std::vector<Vector3f>& normals,
			const std::vector<unsigned>& indices);
		
		~TrianglesMesh() = default;
		
		auto triangle(const size_t index) const -> TriangleF;

		auto size() const noexcept -> size_t;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		size_t mCurrentTriangle = 0;
	};
	
}