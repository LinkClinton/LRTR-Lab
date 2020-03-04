#pragma once

#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Runtimes/RuntimeSharing.hpp"
#include "../../Shared/Math/Math.hpp"
#include "../Workflow.hpp"

#include <memory>

namespace LRTR {

	struct GaussianBlurInput {
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> CommandList;

		std::shared_ptr<CodeRed::GpuFrameBuffer> Output;
		std::shared_ptr<CodeRed::GpuFrameBuffer> Input;

		std::shared_ptr<RuntimeSharing> Sharing;
		
		size_t Times = 5;

		GaussianBlurInput() = default;

		GaussianBlurInput(
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& output,
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& input,
			const std::shared_ptr<RuntimeSharing>& sharing,
			const size_t times = 5) :
			CommandList(commandList), Output(output), Input(input), Sharing(sharing), Times(times) {}
	};

	struct GaussianBlurOutput {
		
	};

	class GaussianBlurWorkflow : public Workflow<GaussianBlurInput, GaussianBlurOutput, false> {
	public:
		explicit GaussianBlurWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device);

		~GaussianBlurWorkflow() = default;
	protected:
		auto work(const WorkflowStartup<GaussianBlurInput>& startup) -> GaussianBlurOutput override;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;

		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;
		
		std::shared_ptr<CodeRed::GpuShaderState> mVertShader;
		std::shared_ptr<CodeRed::GpuShaderState> mFragShader;

		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;

		std::array<std::shared_ptr<CodeRed::GpuDescriptorHeap>, 2> mDescriptorHeaps;
		
		std::shared_ptr<CodeRed::GpuBuffer> mViewBuffer;

		std::shared_ptr<CodeRed::GpuSampler> mSampler;
	};
	
}
