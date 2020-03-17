#pragma once

#include "../../Workflow/Shadow/PointShadowMapWorkflow.hpp"
#include "../../Workflow/PBR/DeferredShadingWorkflow.hpp"

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Shared/Accelerators/Group.hpp"

#include "../System.hpp"

namespace LRTR {

	struct EnvironmentLight {
		std::shared_ptr<CodeRed::GpuTexture> Irradiance;
		std::shared_ptr<CodeRed::GpuTexture> PreFiltering;
		std::shared_ptr<CodeRed::GpuTexture> PreComputingBRDF;

		EnvironmentLight() = default;
		
		EnvironmentLight(
			const std::shared_ptr<CodeRed::GpuTexture>& irradiance,
			const std::shared_ptr<CodeRed::GpuTexture>& preFiltering,
			const std::shared_ptr<CodeRed::GpuTexture>& preComputingBRDF) :
			Irradiance(irradiance), PreFiltering(preFiltering), PreComputingBRDF(preComputingBRDF) {}
	};

	struct LightShadowArea {
		Vector3f Position;
		float Radius = 100.0f;

		LightShadowArea() = default;

		LightShadowArea(
			const Vector3f& position,
			const float& radius) :
			Position(position), Radius(radius) {}
	};

	struct PointShadowMap {
		std::vector<PointShadowFrameBuffer> FrameBuffers;
		std::shared_ptr<CodeRed::GpuTexture> Texture;

		PointShadowMap(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device, const size_t extent, const size_t length);

		~PointShadowMap() = default;
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
			const std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>& commandLists, 
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
			const std::shared_ptr<SceneCamera>& camera, 
			float delta) override;

		void setEnvironmentLight(const EnvironmentLight& light);
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	private:
		void updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const;

		void updateCamera(const std::shared_ptr<SceneCamera>& camera) const;

		auto getCameraPosition(const std::shared_ptr<SceneCamera>& camera) const -> Vector3f;

		auto hasEnvironmentLight() const noexcept -> bool;
	private:
		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::GpuDescriptorHeap> mDescriptorHeap;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;
		std::shared_ptr<CodeRed::GpuBuffer> mViewBuffer;
		std::shared_ptr<CodeRed::GpuSampler> mSampler;

		std::shared_ptr<PointShadowMap> mPointShadowMap;

		std::shared_ptr<DeferredShadingWorkflow> mDeferredShadingWorkflow;
		std::shared_ptr<PointShadowMapWorkflow> mPointShadowMapWorkflow;
		
		std::vector<PointShadowArea> mPointShadowAreas;
		std::vector<PhysicalBasedDrawCall> mDrawCalls;
		std::vector<ShadowCastInfo> mShadowCastInfos;

		DeferredShadingBuffer mDeferredShadingBuffer;

		EnvironmentLight mEnvironmentLight;

		size_t mLights = 0;
	};
	
}