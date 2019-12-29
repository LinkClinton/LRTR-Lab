#pragma once

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Shared/Accelerators/Group.hpp"

#include "../System.hpp"

namespace LRTR {

	class TrianglesMesh;
	
	struct PhysicalBasedDrawCall {
		std::shared_ptr<TrianglesMesh> Mesh;

		unsigned HasBaseColor = 0;
		unsigned HasRoughness = 0;
		unsigned HasOcclusion = 0;
		unsigned HasNormalMap = 0;
		unsigned HasMetallic = 0;
		unsigned HasEmissive = 0;
	};
	
	class PhysicalBasedRenderSystem : public RenderSystem {
	public:
		explicit PhysicalBasedRenderSystem(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			size_t maxFrameCount = 2);

		void update(
			const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta) override;

		void render(
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList, 
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
			const std::shared_ptr<SceneCamera>& camera, 
			float delta) override;

		void setIrradiance(const std::shared_ptr<CodeRed::GpuTexture>& map);
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	private:
		void updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const;

		void updateCamera(const std::shared_ptr<SceneCamera>& camera) const;

		auto getCameraPosition(const std::shared_ptr<SceneCamera>& camera) const -> Vector3f;
	private:
		std::shared_ptr<CodeRed::GpuTexture> mIrradianceMap;
		
		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;
		std::shared_ptr<CodeRed::GpuBuffer> mViewBuffer;
		std::shared_ptr<CodeRed::GpuSampler> mSampler;
		
		std::vector<PhysicalBasedDrawCall> mDrawCalls;
		size_t mLights = 0;
	};
	
}