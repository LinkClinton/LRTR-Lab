#pragma once

#include "../../Shared/Graphics/PipelineInfo.hpp"

#include "../System.hpp"

namespace LRTR {

	class CoordinateRenderSystem : public RenderSystem {
	public:
		explicit CoordinateRenderSystem(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			size_t maxFrameCount = 2);

		void update(
			const StringGroup<std::shared_ptr<Shape>>& shapes, float delta) override;
		
		void render(
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList, 
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
			const std::shared_ptr<SceneCamera>& camera,
			const StringGroup<std::shared_ptr<Shape>>& shapes, 
			float delta) override;
	private:
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;

		std::shared_ptr<CodeRed::GpuBuffer> mAxisVertexBuffer;
		std::shared_ptr<CodeRed::GpuBuffer> mAxisIndexBuffer;
		std::shared_ptr<CodeRed::GpuBuffer> mAxisViewBuffer;

		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
	};
}