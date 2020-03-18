#pragma once

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Runtimes/RuntimeSharing.hpp"

#include "DeferredShadingWorkflow.hpp"
#include "../Workflow.hpp"

#include <memory>

namespace LRTR {

	struct ScreenSpaceAmbientOcclusionBuffer {
		std::shared_ptr<CodeRed::GpuTexture> AmbientOcclusionBlurred;
		std::shared_ptr<CodeRed::GpuTexture> AmbientOcclusion;

		std::shared_ptr<CodeRed::GpuFrameBuffer> FrameBufferBlurred;
		std::shared_ptr<CodeRed::GpuFrameBuffer> FrameBuffer;
		
		ScreenSpaceAmbientOcclusionBuffer() = default;

		void update(
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer);
	};

	using SSAOBuffer = ScreenSpaceAmbientOcclusionBuffer;

	struct ScreenSpaceAmbientOcclusionInput {
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> CommandList;

		std::shared_ptr<RuntimeSharing> Sharing;

		ScreenSpaceAmbientOcclusionBuffer SSAOBuffer;
		DeferredShadingBuffer DSBuffer;

		Matrix4x4f ProjectionMatrix;
		Matrix4x4f ViewMatrix;
		
		ScreenSpaceAmbientOcclusionInput() = default;

		ScreenSpaceAmbientOcclusionInput(
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
			const std::shared_ptr<RuntimeSharing>& sharing,
			const ScreenSpaceAmbientOcclusionBuffer& ssaoBuffer,
			const DeferredShadingBuffer& dsBuffer,
			const Matrix4x4f& projectionMatrix,
			const Matrix4x4f& viewMatrix) :
			CommandList(commandList), Sharing(sharing), SSAOBuffer(ssaoBuffer), DSBuffer(dsBuffer),
			ProjectionMatrix(projectionMatrix), ViewMatrix(viewMatrix) {}
	};

	struct ScreenSpaceAmbientOcclusionOutput {
		ScreenSpaceAmbientOcclusionOutput() = default;
	};

	using SSAOInput = ScreenSpaceAmbientOcclusionInput;
	using SSAOOutput = ScreenSpaceAmbientOcclusionOutput;

	class ScreenSpaceAmbientOcclusionWorkflow : public Workflow<SSAOInput, SSAOOutput, false> {
	public:
		explicit ScreenSpaceAmbientOcclusionWorkflow(
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device);
	protected:
		auto work(const WorkflowStartup<ScreenSpaceAmbientOcclusionInput>& startup) -> ScreenSpaceAmbientOcclusionOutput override;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;

		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;

		std::shared_ptr<CodeRed::GpuShaderState> mVertShader;
		std::shared_ptr<CodeRed::GpuShaderState> mFragShader;

		std::shared_ptr<CodeRed::GpuTextureBuffer> mNoiseTextureBuffer;
		std::shared_ptr<CodeRed::GpuTexture> mNoiseTexture;
		std::shared_ptr<CodeRed::GpuBuffer> mSampleBuffer;
		std::shared_ptr<CodeRed::GpuBuffer> mViewBuffer;

		std::shared_ptr<CodeRed::GpuSampler> mClampSampler;
		std::shared_ptr<CodeRed::GpuSampler> mSampler;
		
		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::GpuDescriptorHeap> mDescriptorHeap;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;

		std::vector<Vector4f> mNoiseVectors;
		std::vector<Vector4f> mSamples;
	};
}
