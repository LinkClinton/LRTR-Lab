#pragma once

#include <CodeRed/Interface/GpuResource/GpuTexture.hpp>
#include <CodeRed/Interface/GpuCommandAllocator.hpp>
#include <CodeRed/Interface/GpuCommandQueue.hpp>

#include "../Workflow.hpp"

#include <string>
#include <memory>

namespace LRTR {

	struct ImageBasedLightingInput {
		std::string FileName;

		size_t EnvironmentMapSize = 512;
		size_t IrradianceMapSize = 32;
		size_t PreFilteringMapSize = 128;
		size_t PreFilteringMipLevels = 5;
		size_t PreComputingBRDFSize = 512;

		std::shared_ptr<CodeRed::GpuCommandQueue> Queue;

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

		std::string mSha256Key;
	};
	
}