#include "BoxMesh.hpp"

LRTR::BoxMesh::BoxMesh(const float width, const float height, const float depth) :
	mWidth(width), mHeight(height), mDepth(depth)
{
	const auto w2 = 0.5f * mWidth;
	const auto h2 = 0.5f * mHeight;
	const auto d2 = 0.5f * mDepth;

	mPositions = {
		Vector3f(-w2, -h2, -d2), Vector3f(-w2, +h2, -d2), Vector3f(+w2, +h2, -d2),
		Vector3f(+w2, -h2, -d2), Vector3f(-w2, -h2, +d2), Vector3f(+w2, -h2, +d2),
		Vector3f(+w2, +h2, +d2), Vector3f(-w2, +h2, +d2), Vector3f(-w2, +h2, -d2),
		Vector3f(-w2, +h2, +d2), Vector3f(+w2, +h2, +d2), Vector3f(+w2, +h2, -d2),
		Vector3f(-w2, -h2, -d2), Vector3f(+w2, -h2, -d2), Vector3f(+w2, -h2, +d2),
		Vector3f(-w2, -h2, +d2), Vector3f(-w2, -h2, +d2), Vector3f(-w2, +h2, +d2),
		Vector3f(-w2, +h2, -d2), Vector3f(-w2, -h2, -d2), Vector3f(+w2, -h2, -d2),
		Vector3f(+w2, +h2, -d2), Vector3f(+w2, +h2, +d2), Vector3f(+w2, -h2, +d2)
	};

	mNormals = {
		Vector3f(+0.0f, +0.0f, -1.0f), Vector3f(+0.0f, +0.0f, -1.0f), Vector3f(+0.0f, +0.0f, -1.0f),
		Vector3f(+0.0f, +0.0f, -1.0f), Vector3f(+0.0f, +0.0f, +1.0f), Vector3f(+0.0f, +0.0f, +1.0f),
		Vector3f(+0.0f, +0.0f, +1.0f), Vector3f(+0.0f, +0.0f, +1.0f), Vector3f(+0.0f, +1.0f, +0.0f),
		Vector3f(+0.0f, +1.0f, +0.0f), Vector3f(+0.0f, +1.0f, +0.0f), Vector3f(+0.0f, +1.0f, +0.0f),
		Vector3f(+0.0f, -1.0f, +0.0f), Vector3f(+0.0f, -1.0f, +0.0f), Vector3f(+0.0f, -1.0f, +0.0f),
		Vector3f(+0.0f, -1.0f, +0.0f), Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(-1.0f, +0.0f, +0.0f),
		Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f),
		Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f)
	};

	mTangents = {
		Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f),
		Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(-1.0f, +0.0f, +0.0f),
		Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f),
		Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f),
		Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(-1.0f, +0.0f, +0.0f),
		Vector3f(-1.0f, +0.0f, +0.0f), Vector3f(+0.0f, +0.0f, -1.0f), Vector3f(+0.0f, +0.0f, -1.0f),
		Vector3f(+0.0f, +0.0f, -1.0f), Vector3f(+0.0f, +0.0f, -1.0f), Vector3f(+0.0f, +0.0f, +1.0f),
		Vector3f(+0.0f, +0.0f, +1.0f), Vector3f(+0.0f, +0.0f, +1.0f), Vector3f(+0.0f, +0.0f, +1.0f)
	};

	mTexCoords = {
		Vector3f(+0.0f, +1.0f, +0.0f), Vector3f(+0.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f),
		Vector3f(+1.0f, +1.0f, +0.0f), Vector3f(+1.0f, +1.0f, +0.0f), Vector3f(+0.0f, +1.0f, +0.0f),
		Vector3f(+0.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+0.0f, +1.0f, +0.0f),
		Vector3f(+0.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +1.0f, +0.0f),
		Vector3f(+1.0f, +1.0f, +0.0f), Vector3f(+0.0f, +1.0f, +0.0f), Vector3f(+0.0f, +0.0f, +0.0f),
		Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+0.0f, +1.0f, +0.0f), Vector3f(+0.0f, +0.0f, +0.0f),
		Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +1.0f, +0.0f), Vector3f(+0.0f, +1.0f, +0.0f),
		Vector3f(+0.0f, +0.0f, +0.0f), Vector3f(+1.0f, +0.0f, +0.0f), Vector3f(+1.0f, +1.0f, +0.0f),
	};

	mIndices = {
		0, 1, 2, 0, 2, 3,
		4, 5 ,6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};
}

auto LRTR::BoxMesh::typeName() const noexcept -> std::string
{
	return "BoxMesh";
}

auto LRTR::BoxMesh::typeIndex() const noexcept -> std::type_index
{
	return typeid(BoxMesh);
}

void LRTR::BoxMesh::onProperty()
{
	TrianglesMesh::onProperty();
}
