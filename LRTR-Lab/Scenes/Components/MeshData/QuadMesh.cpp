#include "QuadMesh.hpp"

LRTR::QuadMesh::QuadMesh(const float width, const float height) :
	mWidth(width), mHeight(height)
{
	const auto hWidth = width * 0.5f;
	const auto hHeight = height * 0.5f;

	mPositions = {
		Vector3f(-hWidth, -hHeight, 0.0f),
		Vector3f(-hWidth, +hHeight, 0.0f),
		Vector3f(+hWidth, +hHeight, 0.0f),
		Vector3f(+hWidth, -hHeight, 0.0f)
	};

	mTexCoords = {
		Vector3f(0.0f, 0.0f, 0.0f),
		Vector3f(0.0f, 1.0f, 0.0f),
		Vector3f(1.0f, 1.0f, 0.0f),
		Vector3f(1.0f, 0.0f, 0.0f)
	};

	mIndices = {
		0, 1, 2,
		0, 2, 3
	};
	
	mNormals = std::vector<Vector3f>(4);
	mTangents = std::vector<Vector3f>(4);
}

auto LRTR::QuadMesh::typeName() const noexcept -> std::string
{
	return "QuadMesh";
}

auto LRTR::QuadMesh::typeIndex() const noexcept -> std::type_index
{
	return typeid(QuadMesh);
}

void LRTR::QuadMesh::onProperty()
{
	TrianglesMesh::onProperty();
}
