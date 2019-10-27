#pragma once

#include "UIComponents/UIComponent.hpp"
#include "../Layer.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

#include <memory>

namespace LRTR {

	class UILayer : public Layer {
	public:
		explicit UILayer(
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			const std::shared_ptr<CodeRed::GpuRenderPass>& renderPass,
			const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator,
			const std::shared_ptr<CodeRed::GpuCommandQueue>& queue);

		void update(float delta) override;
		
		void render(
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer,
			float delta);
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;
		
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> mCommandList;
		std::shared_ptr<CodeRed::GpuCommandAllocator> mCommandAllocator;
		std::shared_ptr<CodeRed::GpuCommandQueue> mCommandQueue;

		std::shared_ptr<CodeRed::ImGuiWindows> mImGuiWindows;

		std::unordered_map<std::string, std::shared_ptr<UIComponent>> mUIComponents;
	};

}