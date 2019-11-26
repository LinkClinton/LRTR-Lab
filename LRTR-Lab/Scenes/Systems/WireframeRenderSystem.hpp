#pragma once

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Shared/Accelerators/Group.hpp"

#include "../System.hpp"

namespace LRTR {

	class TrianglesMesh;
	
	class WireframeRenderSystem : public RenderSystem {
	public:
		explicit WireframeRenderSystem(
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
		using Location = std::pair<size_t, size_t>;
		using DataIndexGroup = Group<std::shared_ptr<TrianglesMesh>, Location>;
		
		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;
		std::shared_ptr<CodeRed::GpuBuffer> mViewBuffer;
	};
	
}