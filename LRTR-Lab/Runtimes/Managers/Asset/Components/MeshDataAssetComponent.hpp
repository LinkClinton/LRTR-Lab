#pragma once

#include "../../../../Scenes/Components/MeshData/MeshData.hpp"
#include "../../../../Shared/Accelerators/Group.hpp"
#include "AssetComponent.hpp"

namespace LRTR {

	struct MeshDataInfo {
		size_t StartVertexLocation;
		size_t StartIndexLocation;
		size_t IndexCount;
	};

	class MeshDataAssetComponent : public AssetComponent {
	public:
		explicit MeshDataAssetComponent(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device);

		~MeshDataAssetComponent() = default;

		void beginAllocating();

		void endAllocating();

		void allocate(const std::shared_ptr<MeshData>& meshData);
		
		auto get(const std::shared_ptr<MeshData>& meshData) -> MeshDataInfo;
		
		auto positions() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>;

		auto texCoords() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>;

		auto tangents() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>;

		auto normals() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>;

		auto indices() const noexcept -> std::shared_ptr<CodeRed::GpuBuffer>;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		std::shared_ptr<CodeRed::GpuBuffer> mIndices;
		std::vector<std::shared_ptr<CodeRed::GpuBuffer>> mProperties;

		std::vector<std::vector<Vector3f>> mPropertiesAllocateCache;
		std::vector<unsigned> mIndicesAllocateCache;
		size_t mVertexLocation = 0;
		size_t mIndexLocation = 0;
		
		Group<Identity, MeshDataInfo> mMeshDataInfos;
	};
	
}