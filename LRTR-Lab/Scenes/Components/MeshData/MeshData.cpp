#include "MeshData.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::MeshData::MeshData(
	const std::vector<Vector3f>& vertices, 
	const std::vector<unsigned>& indices,
	const CodeRed::PrimitiveTopology primitive) :
	MeshData(
		std::vector<Vector3f>(),
		vertices,
		std::vector<Vector3f>(),
		std::vector<Vector3f>(),
		indices,
		primitive)
{
}

LRTR::MeshData::MeshData(
	const std::vector<Vector3f>& texCoords, 
	const std::vector<Vector3f>& vertices,
	const std::vector<Vector3f>& tangents, 
	const std::vector<Vector3f>& normals, 
	const std::vector<unsigned>& indices,
	const CodeRed::PrimitiveTopology primitive) :
	mTexCoords(texCoords), mVertices(vertices), mTangents(tangents),
	mNormals(normals), mIndices(indices), mPrimitive(primitive)
{
}

auto LRTR::MeshData::texCoords() const noexcept -> const std::vector<Vector3f>& 
{
	return mTexCoords;
}

auto LRTR::MeshData::vertices() const noexcept -> const std::vector<Vector3f>& 
{
	return mVertices;
}

auto LRTR::MeshData::tangents() const noexcept -> const std::vector<Vector3f>& 
{
	return mTangents;
}

auto LRTR::MeshData::normals() const noexcept -> const std::vector<Vector3f>& 
{
	return mNormals;
}

auto LRTR::MeshData::indices() const noexcept -> const std::vector<unsigned>& 
{
	return mIndices;
}

auto LRTR::MeshData::primitive() const noexcept -> CodeRed::PrimitiveTopology
{
	return mPrimitive;
}

auto LRTR::MeshData::typeName() const noexcept -> std::string
{
	return "MeshData";
}

auto LRTR::MeshData::typeIndex() const noexcept -> std::type_index
{
	return typeid(MeshData);
}

void LRTR::MeshData::onProperty()
{
	ImGui::BeginPropertyTable("Property");

	ImGui::Property("Vertices", [&]()
		{
			ImGui::Text(std::to_string(mVertices.size()).c_str());
		});

	ImGui::Property("Indices", [&]()
		{
			ImGui::Text(std::to_string(mIndices.size()).c_str());
		});

	ImGui::EndPropertyTable();
}
