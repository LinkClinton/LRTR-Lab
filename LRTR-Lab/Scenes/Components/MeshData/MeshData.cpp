#include "MeshData.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::MeshData::MeshData(
	const std::vector<Vector3f>& positions, 
	const std::vector<unsigned>& indices,
	const CodeRed::PrimitiveTopology primitive) :
	MeshData(
		positions,
		std::vector<Vector3f>(),
		std::vector<Vector3f>(),
		std::vector<Vector3f>(),
		indices,
		primitive)
{
}

LRTR::MeshData::MeshData(
	const std::vector<Vector3f>& positions,
	const std::vector<Vector3f>& texCoords,
	const std::vector<Vector3f>& tangents, 
	const std::vector<Vector3f>& normals, 
	const std::vector<unsigned>& indices,
	const CodeRed::PrimitiveTopology primitive) :
	mPositions(positions), mTexCoords(texCoords), mTangents(tangents),
	mNormals(normals), mIndices(indices), mPrimitive(primitive)
{
	LRTR_WARNING_IF(mPositions.size() > mTexCoords.size() && !mTexCoords.empty(),
		"The size of texcoords must greater or equal than size of positions.");
	LRTR_WARNING_IF(mPositions.size() > mTangents.size() && !mTangents.empty(),
		"The size of tangents must greater or equal than size of positions.");
	LRTR_WARNING_IF(mPositions.size() > mNormals.size() && !mNormals.empty(),
		"The size of normals must greater or equal than size of positions.");
}

auto LRTR::MeshData::positions() const noexcept -> const std::vector<Vector3f>& 
{
	return mPositions;
}

auto LRTR::MeshData::texCoords() const noexcept -> const std::vector<Vector3f>& 
{
	return mTexCoords;
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
			ImGui::Text(std::to_string(mPositions.size()).c_str());
		});

	ImGui::Property("Indices", [&]()
		{
			ImGui::Text(std::to_string(mIndices.size()).c_str());
		});

	ImGui::EndPropertyTable();
}
