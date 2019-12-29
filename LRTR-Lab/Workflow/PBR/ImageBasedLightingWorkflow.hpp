#pragma once

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Runtimes/RuntimeSharing.hpp"
#include "../Workflow.hpp"

#include <string>
#include <memory>

namespace LRTR {

	struct ImageBasedLightingInput {
		std::string FileName = "";

		size_t EnvironmentMapSize = 512;
		size_t IrradianceMapSize = 32;
		size_t PreFilteringMapSize = 128;
		size_t PreFilteringMipLevels = 5;
		size_t PreComputingBRDFSize = 512;

		std::shared_ptr<CodeRed::GpuCommandQueue> Queue = nullptr;
		std::shared_ptr<RuntimeSharing> mRuntimeSharing = nullptr;
		
		ImageBasedLightingInput() = default;

		ImageBasedLightingInput(
			const std::shared_ptr<CodeRed::GpuCommandQueue> queue,
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::string fileName) :
			FileName(fileName), Queue(queue), mRuntimeSharing(sharing) {}
		
		auto string() const noexcept -> std::string;
	};

	struct ImageBasedLightingOutput {
		std::shared_ptr<CodeRed::GpuTexture> EnvironmentMap;
		std::shared_ptr<CodeRed::GpuTexture> IrradianceMap;
		std::shared_ptr<CodeRed::GpuTexture> PreFilteringMap;
		std::shared_ptr<CodeRed::GpuTexture> PreComputingBRDF;
	};

	using IBLInput = ImageBasedLightingInput;
	using IBLOutput = ImageBasedLightingOutput;

	class ImageBasedLightingWorkflow : public Workflow<IBLInput, IBLOutput> {
	public:
		explicit ImageBasedLightingWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device);

		~ImageBasedLightingWorkflow() = default;
	protected:
		auto readCache(const WorkflowStartup<ImageBasedLightingInput>& startup)
			-> std::optional<ImageBasedLightingOutput> override;

		void writeCache(
			const WorkflowStartup<ImageBasedLightingInput>& startup, 
			const ImageBasedLightingOutput& output) override;

		auto work(const WorkflowStartup<ImageBasedLightingInput>& startup) -> ImageBasedLightingOutput override;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		std::shared_ptr<CodeRed::GpuCommandAllocator> mAllocator;

		std::shared_ptr<CodeRed::GpuShaderState> mEnvironmentMapVShader;
		std::shared_ptr<CodeRed::GpuShaderState> mEnvironmentMapFShader;
		std::shared_ptr<CodeRed::GpuRenderPass> mEnvironmentMapRenderPass;
		
		std::shared_ptr<CodeRed::GpuDescriptorHeap> mDescriptorHeap;
		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;

		std::shared_ptr<CodeRed::GpuBuffer> mViewBuffer;
		std::shared_ptr<CodeRed::GpuSampler> mSampler;
		
		std::string mSha256Key;
	};
	
}