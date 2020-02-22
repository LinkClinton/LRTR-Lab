#pragma once

#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Runtimes/RuntimeSharing.hpp"
#include "../../Shared/Math/Math.hpp"
#include "../Workflow.hpp"

#include <memory>

namespace LRTR {

	struct ShadowCastInfo {
		std::shared_ptr<TrianglesMesh> Mesh;
		size_t Index = 0;

		ShadowCastInfo() = default;

		ShadowCastInfo(
			const std::shared_ptr<TrianglesMesh>& mesh,
			const size_t& index) : Mesh(mesh), Index(index) {}
	};
	
	struct PointShadowMapInput {
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> CommandList;
		std::shared_ptr<CodeRed::GpuTexture> ShadowMap;
		std::shared_ptr<CodeRed::GpuBuffer> Transform;

		std::shared_ptr<RuntimeSharing> Sharing;

		std::vector<ShadowCastInfo> Info;
		
		Vector3f Position = Vector3f(0);
		float Radius = 100;

		PointShadowMapInput() = default;

		PointShadowMapInput(
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
			const std::shared_ptr<CodeRed::GpuTexture>& shadowMap,
			const std::shared_ptr<CodeRed::GpuBuffer>& transform,
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::vector<ShadowCastInfo>& info,
			const Vector3f& position,
			const float radius = 100) :
			CommandList(commandList), ShadowMap(shadowMap), Transform(transform),
			Sharing(sharing), Info(info), Position(position), Radius(radius) {}
	};

	struct PointShadowMapOutput {
		std::shared_ptr<CodeRed::GpuTexture> ShadowMap;

		PointShadowMapOutput() = default;
	};


	class PointShadowMapWorkflow : public Workflow<PointShadowMapInput, PointShadowMapOutput, false> {
	public:
		explicit PointShadowMapWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device);

		~PointShadowMapWorkflow() = default;
	protected:
		auto work(const WorkflowStartup<PointShadowMapInput>& startup) -> PointShadowMapOutput override;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;

		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;

		std::shared_ptr<CodeRed::GpuShaderState> mVertShader;
		std::shared_ptr<CodeRed::GpuShaderState> mFragShader;

		std::shared_ptr<CodeRed::GpuFrameBuffer> mFrameBuffer[6];
		
		std::shared_ptr<CodeRed::GpuDescriptorHeap> mDescriptorHeap;
		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;

		std::shared_ptr<CodeRed::GpuBuffer> mViewBuffer;
	};

}