#pragma once

#include "../../Shared/Graphics/PipelineInfo.hpp"
#include "../../Shared/Accelerators/Group.hpp"
#include "../../Shared/Color.hpp"

#include "../System.hpp"

namespace LRTR {

	struct PhysicalBasedDrawCall {
		size_t StartVertexLocation = 0;
		size_t StartIndexLocation = 0;
		size_t IndexCount = 0;

		bool HasBaseColor = false;
		bool HasRoughness = false;
		bool HasOcclusion = false;
		bool HasNormalMap = false;
		bool HasMetallic = false;
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
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList, 
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
			const std::shared_ptr<SceneCamera>& camera, 
			float delta) override;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	private:
		void updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const;

		void updateCamera(const std::shared_ptr<SceneCamera>& camera) const;
	private:
		using Location = std::pair<size_t, size_t>;
		using DataIndexGroup = Group<Identity, Location>;

		std::shared_ptr<CodeRed::GpuResourceLayout> mResourceLayout;
		std::shared_ptr<CodeRed::PipelineInfo> mPipelineInfo;
		std::shared_ptr<CodeRed::GpuBuffer> mViewBuffer;

		std::vector<PhysicalBasedDrawCall> mDrawCalls;
	};
	
}