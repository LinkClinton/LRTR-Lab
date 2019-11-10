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
			const std::shared_ptr<CodeRed::GpuCommandQueue>& queue,
			const size_t width, const size_t height);

		void update(float delta) override;
		
		void render(
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer,
			float delta);

		void resize(const size_t width, const size_t height);
		
		auto width() const noexcept -> size_t;

		auto height() const noexcept -> size_t;
		
		auto components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>&;

		auto device() const noexcept->std::shared_ptr<CodeRed::GpuLogicalDevice>;

		auto commandAllocator() const noexcept->std::shared_ptr<CodeRed::GpuCommandAllocator>;

		auto commandQueue() const noexcept->std::shared_ptr<CodeRed::GpuCommandQueue>;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;
		
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> mCommandList;
		std::shared_ptr<CodeRed::GpuCommandAllocator> mCommandAllocator;
		std::shared_ptr<CodeRed::GpuCommandQueue> mCommandQueue;

		std::shared_ptr<CodeRed::ImGuiWindows> mImGuiWindows;

		StringGroup<std::shared_ptr<UIComponent>> mUIComponents;

		size_t mWidth, mHeight;
	};

	class UILayerSharing final : public Noncopyable {
	public:
		explicit UILayerSharing(UILayer* layer);

		auto components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>&;

		auto width() const noexcept -> size_t;

		auto height() const noexcept -> size_t;

		auto device() const noexcept -> std::shared_ptr<CodeRed::GpuLogicalDevice>;

		auto commandAllocator() const noexcept -> std::shared_ptr<CodeRed::GpuCommandAllocator>;

		auto commandQueue() const noexcept -> std::shared_ptr<CodeRed::GpuCommandQueue>;
	private:
		UILayer* mLayer = nullptr;
	};
}