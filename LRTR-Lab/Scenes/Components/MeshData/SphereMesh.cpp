#include "SphereMesh.hpp"

LRTR::SphereMesh::SphereMesh(const float radius, const int slice, const int stack) :
	mRadius(radius)
{
	mPositions.push_back(Vector3f(0.0f, +mRadius, 0.0f));
	mTexCoords.push_back(Vector3f(0.0f, 0.0f, 0.0f));
	mTangents.push_back(Vector3f(0.0f, 0.0f, 0.0f));
	mNormals.push_back(Vector3f(0.0f, +1.0f, 0.0f));

	const auto phiStep = MathUtility::pi<float>() / stack;
	const auto thetaStep = MathUtility::two_pi<float>() / slice;

	for (size_t index0 = 1; index0 < stack; index0++) {
		const auto phi = index0 * phiStep;

		for (size_t index1 = 0; index1 <= slice; index1++) {
			const auto theta = index1 * thetaStep;

			mPositions.push_back(Vector3f(
				mRadius * MathUtility::sin(phi) * MathUtility::cos(theta),
				mRadius * MathUtility::cos(phi),
				mRadius * MathUtility::sin(phi) * MathUtility::sin(theta)
			));

			mTexCoords.push_back(Vector3f(
				theta / MathUtility::two_pi<float>() * 4,
				phi / MathUtility::pi<float>() * 2,
				0.0f
			));

			mTangents.push_back(MathUtility::normalize(Vector3f(
				-mRadius * MathUtility::sin(phi) * MathUtility::sin(theta),
				0.0f,
				+mRadius * MathUtility::sin(phi) * MathUtility::cos(theta)
			)));

			mNormals.push_back(MathUtility::normalize(mPositions[mPositions.size() - 1]));
		}
	}
	
	mPositions.push_back(Vector3f(0.0f, -mRadius, 0.0f));
	mTexCoords.push_back(Vector3f(0.0f, 1.0f, 0.0f));
	mTangents.push_back(Vector3f(0.0f, 0.0f, 0.0f));
	mNormals.push_back(Vector3f(0.0f, -1.0f, 0.0f));

	for (size_t index = 1; index <= slice; index++) {
		mIndices.push_back(0);
		mIndices.push_back(static_cast<unsigned>(index + 1));
		mIndices.push_back(static_cast<unsigned>(index));
	}

	size_t baseIndex = 1;
	size_t ringVertexCount = slice + 1;

	for (size_t index0 = 0; index0 < stack - 2; index0++) {
		for (size_t index1 = 0; index1 < slice; index1++) {
			mIndices.push_back(static_cast<unsigned>(baseIndex + index0 * ringVertexCount + index1));
			mIndices.push_back(static_cast<unsigned>(baseIndex + index0 * ringVertexCount + index1 + 1));
			mIndices.push_back(static_cast<unsigned>(baseIndex + (index0 + 1) * ringVertexCount + index1));

			mIndices.push_back(static_cast<unsigned>(baseIndex + (index0 + 1) * ringVertexCount + index1));
			mIndices.push_back(static_cast<unsigned>(baseIndex + index0 * ringVertexCount + index1 + 1));
			mIndices.push_back(static_cast<unsigned>(baseIndex + (index0 + 1) * ringVertexCount + index1 + 1));
		}
	}

	size_t southPoleIndex = mPositions.size() - 1;

	baseIndex = southPoleIndex - ringVertexCount;

	for (size_t index = 0; index < slice; index++) {
		mIndices.push_back(static_cast<unsigned>(southPoleIndex));
		mIndices.push_back(static_cast<unsigned>(baseIndex + index));
		mIndices.push_back(static_cast<unsigned>(baseIndex + index + 1));
	}
}

auto LRTR::SphereMesh::typeName() const noexcept -> std::string
{
	return "SphereMesh";
}

auto LRTR::SphereMesh::typeIndex() const noexcept -> std::type_index
{
	return typeid(SphereMesh);
}

void LRTR::SphereMesh::onProperty()
{
	TrianglesMesh::onProperty();
}
