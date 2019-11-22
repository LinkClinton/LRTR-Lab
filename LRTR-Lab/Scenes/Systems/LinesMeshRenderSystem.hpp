#pragma once

#include "../../Shared/Graphics/PipelineInfo.hpp"

#include "../System.hpp"

namespace LRTR {

	class LinesMeshRenderSystem : public RenderSystem {
	public:
		explicit LinesMeshRenderSystem(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			size_t maxFrameCount = 2);

		void update(
			const StringGroup<std::shared_ptr<Shape>>& shapes, float delta) override;

		void render(
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer,
			const std::shared_ptr<SceneCamera>& camera,
			float delta) override;
	private:
		void updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const;

		void updateCamera(const std::shared_ptr<SceneCamera>& camera) const;
	private:
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;

		std::shared_ptr<CodeRed::GpuBuffer> mLineViewBuffer;

		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;

		size_t mIndexCount = 0;
	};
}