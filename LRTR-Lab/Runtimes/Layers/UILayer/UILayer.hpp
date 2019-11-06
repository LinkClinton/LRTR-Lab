#pragma once

#include "../../../Shared/Accelerators/Group.hpp"
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

		auto components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>&;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;
		
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> mCommandList;
		std::shared_ptr<CodeRed::GpuCommandAllocator> mCommandAllocator;
		std::shared_ptr<CodeRed::GpuCommandQueue> mCommandQueue;

		std::shared_ptr<CodeRed::ImGuiWindows> mImGuiWindows;

		StringGroup<std::shared_ptr<UIComponent>> mUIComponents;
	};

	class UILayerSharing final : public Noncopyable {
	public:
		explicit UILayerSharing(UILayer* layer);

		auto components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>&;
	private:
		UILayer* mLayer = nullptr;
	};
}