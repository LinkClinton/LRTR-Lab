#pragma once

#include <CodeRed/Shared/Enum/PrimitiveTopology.hpp>

#include "../../../Shared/Math/Math.hpp"
#include "../../Component.hpp"

namespace LRTR {

	class MeshData : public Component {
	public:
		MeshData() = default;

		explicit MeshData(
			const std::vector<Vector3f>& vertices,
			const std::vector<unsigned>& indices,
			const CodeRed::PrimitiveTopology primitive = CodeRed::PrimitiveTopology::TriangleList);

		explicit MeshData(
			const std::vector<Vector3f>& texCoords,
			const std::vector<Vector3f>& vertices,
			const std::vector<Vector3f>& tangents,
			const std::vector<Vector3f>& normals,
			const std::vector<unsigned>& indices,
			const CodeRed::PrimitiveTopology primitive = CodeRed::PrimitiveTopology::TriangleList);
		
		auto texCoords() const noexcept -> const std::vector<Vector3f>&;
		
		auto vertices() const noexcept -> const std::vector<Vector3f>&;

		auto tangents() const noexcept -> const std::vector<Vector3f>&;

		auto normals() const noexcept -> const std::vector<Vector3f>&;

		auto indices() const noexcept -> const std::vector<unsigned>&;

		auto primitive() const noexcept -> CodeRed::PrimitiveTopology;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	protected:
		std::vector<Vector3f> mTexCoords;
		std::vector<Vector3f> mVertices;
		std::vector<Vector3f> mTangents;
		std::vector<Vector3f> mNormals;
		std::vector<unsigned> mIndices;

		CodeRed::PrimitiveTopology mPrimitive = CodeRed::PrimitiveTopology::TriangleList;
	};
	
}