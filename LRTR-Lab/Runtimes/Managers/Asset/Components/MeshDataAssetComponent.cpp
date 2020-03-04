#include "MeshDataAssetComponent.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../../../Scenes/Components/MeshData/SphereMesh.hpp"
#include "../../../../Scenes/Components/MeshData/QuadMesh.hpp"
#include "../../../../Scenes/Components/MeshData/BoxMesh.hpp"
#include "../../../../Shared/Graphics/ResourceHelper.hpp"

#define LRTR_INSERT_VERTEX_PROPERTY(condition, dest, source0, source1) \
	if (condition) dest.insert(dest.end(), source0.begin(), source0.end()); \
	else dest.insert(dest.end(), source1.begin(), source1.end());

LRTR::MeshDataAssetComponent::MeshDataAssetComponent(
	const std::shared_ptr<RuntimeSharing> & sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice> & device) :
	AssetComponent(sharing), mDevice(device)
{
	mProperties = std::vector<std::shared_ptr<CodeRed::GpuBuffer>>(4);

	for (size_t index = 0; index < mProperties.size(); index++) {
		mProperties[index] = mDevice->createBuffer(
			CodeRed::ResourceInfo::VertexBuffer(
				sizeof(Vector3f),
				10000,
				CodeRed::MemoryHeap::Upload
			)
		);
	}

	mIndices = mDevice->createBuffer(
		CodeRed::ResourceInfo::IndexBuffer(
			sizeof(unsigned),
			30000,
			CodeRed::MemoryHeap::Upload
		)
	);

	beginAllocating();
	
	allocate(mMeshes["SkyBox"] = std::make_shared<BoxMesh>(2.0f, 2.0f, 2.0f));
	allocate(mMeshes["Quad"] = std::make_shared<QuadMesh>(2.0f, 2.0f));
	allocate(mMeshes["Sphere"] = std::make_shared<SphereMesh>(1.0f, 64, 64));
	
	endAllocating();
}

void LRTR::MeshDataAssetComponent::beginAllocating()
{
	mPropertiesAllocateCache = std::vector<std::vector<Vector3f>>(4);
	mIndicesAllocateCache = std::vector<unsigned>();
}

void LRTR::MeshDataAssetComponent::endAllocating()
{
	//expand the buffer if the count is not enough
	for (size_t index = 0; index < mProperties.size(); index++) {
		mProperties[index] = CodeRed::ResourceHelper::expandAndCopyBuffer(mDevice, mProperties[index], mVertexLocation);

		CodeRed::ResourceHelper::updateBuffer(mProperties[index], mPropertiesAllocateCache[index].data(),
			sizeof(Vector3f) * (mVertexLocation - mPropertiesAllocateCache[index].size()),
			sizeof(Vector3f) * mPropertiesAllocateCache[index].size());
	}
	
	mIndices = CodeRed::ResourceHelper::expandAndCopyBuffer(mDevice, mIndices, mIndexLocation);

	CodeRed::ResourceHelper::updateBuffer(mIndices, mIndicesAllocateCache.data(),
		sizeof(unsigned) * (mIndexLocation - mIndicesAllocateCache.size()),
		sizeof(unsigned) * mIndicesAllocateCache.size());
	
	mPropertiesAllocateCache.clear();
	mIndicesAllocateCache.clear();
}

void LRTR::MeshDataAssetComponent::allocate(const std::shared_ptr<MeshData>& meshData)
{
	if (mMeshDataInfos.find(meshData->identity()) != mMeshDataInfos.end()) return;

	//the zero array is used to fill properties that mesh does not have.
	//when we create the mesh, we will test the size of properties,
	//so the size of properties will be greater or equal than positions
	const auto zeroArray = std::vector<Vector3f>(
		meshData->positions().size() > meshData->texCoords().size() ||
		meshData->positions().size() > meshData->tangents().size() ||
		meshData->positions().size() > meshData->normals().size() ?
		meshData->positions().size() : 0, Vector3f());

	LRTR_INSERT_VERTEX_PROPERTY(
		meshData->positions().size() >= meshData->positions().size(),
		mPropertiesAllocateCache[0], meshData->positions(), zeroArray);

	LRTR_INSERT_VERTEX_PROPERTY(
		meshData->positions().size() >= meshData->texCoords().size(),
		mPropertiesAllocateCache[1], meshData->texCoords(), zeroArray);

	LRTR_INSERT_VERTEX_PROPERTY(
		meshData->positions().size() >= meshData->tangents().size(),
		mPropertiesAllocateCache[2], meshData->tangents(), zeroArray);

	LRTR_INSERT_VERTEX_PROPERTY(
		meshData->positions().size() >= meshData->normals().size(),
		mPropertiesAllocateCache[3], meshData->normals(), zeroArray);

	mIndicesAllocateCache.insert(mIndicesAllocateCache.end(),
		meshData->indices().begin(), meshData->indices().end());

	mMeshDataInfos.insert({ meshData->identity(),
		{ mVertexLocation, mIndexLocation, meshData->indices().size() } });
	
	mVertexLocation = mVertexLocation + meshData->positions().size();
	mIndexLocation = mIndexLocation + meshData->indices().size();
}

auto LRTR::MeshDataAssetComponent::get(const std::shared_ptr<MeshData>& meshData) -> MeshDataInfo
{
	assert(mMeshDataInfos.find(meshData->identity()) != mMeshDataInfos.end());

	return mMeshDataInfos[meshData->identity()];
}

auto LRTR::MeshDataAssetComponent::get(const std::string& meshName) -> MeshDataInfo
{
	assert(mMeshes.find(meshName) != mMeshes.end());
	
	return get(mMeshes[meshName]);
}

auto LRTR::MeshDataAssetComponent::positions() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>
{
	return mProperties[0];
}

auto LRTR::MeshDataAssetComponent::texCoords() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>
{
	return mProperties[1];
}

auto LRTR::MeshDataAssetComponent::tangents() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>
{
	return mProperties[2];
}

auto LRTR::MeshDataAssetComponent::normals() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>
{
	return mProperties[3];
}

auto LRTR::MeshDataAssetComponent::indices() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>
{
	return mIndices;
}
