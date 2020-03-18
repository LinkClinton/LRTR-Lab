#pragma once

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Runtimes/RuntimeSharing.hpp"
#include "../Workflow.hpp"

#include <memory>

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
		unsigned HasBlurred = 0;
	};

	struct DeferredShadingBuffer {
		std::shared_ptr<CodeRed::GpuTexture> BaseColorAndRoughness;
		std::shared_ptr<CodeRed::GpuTexture> PositionAndOcclusion;
		std::shared_ptr<CodeRed::GpuTexture> EmissiveAndMetallic;
		std::shared_ptr<CodeRed::GpuTexture> ViewSpacePosition;
		std::shared_ptr<CodeRed::GpuTexture> NormalAndBlur;

		std::shared_ptr<CodeRed::GpuTexture> Depth;
		
		std::shared_ptr<CodeRed::GpuFrameBuffer> FrameBuffer;
		
		DeferredShadingBuffer() = default;

		void update(
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& buffer);
	};
	
	struct DeferredShadingInput {
		std::vector<std::shared_ptr<CodeRed::GpuDescriptorHeap>> DescriptorHeaps;
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> CommandList;
		
		std::shared_ptr<RuntimeSharing> Sharing;

		std::vector<PhysicalBasedDrawCall> DrawCalls;

		DeferredShadingBuffer DeferredShadingBuffer;

		DeferredShadingInput() = default;
		
		DeferredShadingInput(
			const std::vector<std::shared_ptr<CodeRed::GpuDescriptorHeap>>& descriptorHeaps,
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::vector<PhysicalBasedDrawCall>& drawCalls,
			const LRTR::DeferredShadingBuffer& deferredShadingBuffer) :
			DescriptorHeaps(descriptorHeaps), CommandList(commandList), Sharing(sharing), DrawCalls(drawCalls),
			DeferredShadingBuffer(deferredShadingBuffer) {}
	};

	struct DeferredShadingOutput {
		DeferredShadingOutput() = default;
	};

	using DSInput = DeferredShadingInput;
	using DSOutput = DeferredShadingOutput;
	
	class DeferredShadingWorkflow : public Workflow<DSInput, DSOutput, false> {
	public:
		explicit DeferredShadingWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device);

		auto resourceLayout() const noexcept -> std::shared_ptr<CodeRed::GpuResourceLayout>;
	protected:
		auto work(const WorkflowStartup<DeferredShadingInput>& startup) -> DeferredShadingOutput override;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;

		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;

		std::shared_ptr<CodeRed::GpuShaderState> mVertShader;
		std::shared_ptr<CodeRed::GpuShaderState> mFragShader;

		std::shared_ptr<CodeRed::GpuSampler> mSampler;
		
		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;
	};
	
}
