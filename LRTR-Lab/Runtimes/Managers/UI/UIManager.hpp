#pragma once

#include "../../../Shared/Accelerators/Group.hpp"
#include "Components/UIComponent.hpp"
#include "../Manager.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

#include <memory>

namespace LRTR {

	class UIManager : public Manager {
	public:
		explicit UIManager(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			const std::shared_ptr<CodeRed::GpuRenderPass>& renderPass,
			const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator,
			const std::shared_ptr<CodeRed::GpuCommandQueue>& queue,
			const size_t width, const size_t height, const size_t font);

		void update(float delta) override;
		
		auto render(
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, float delta)
			-> std::shared_ptr<CodeRed::GpuGraphicsCommandList>;

		void addComponent(
			const std::string& name,
			const std::shared_ptr<UIComponent>& component);

		void removeComponent(const std::string& name);
		
		void resize(const size_t width, const size_t height);

		auto width() const noexcept->size_t;

		auto height() const noexcept->size_t;

		auto components() const noexcept -> const StringOrderGroup<std::shared_ptr<UIComponent>>&;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;

		std::shared_ptr<CodeRed::GpuGraphicsCommandList> mCommandList;
		std::shared_ptr<CodeRed::GpuCommandAllocator> mCommandAllocator;
		std::shared_ptr<CodeRed::GpuCommandQueue> mCommandQueue;

		std::shared_ptr<CodeRed::ImGuiWindows> mImGuiWindows;

		StringOrderGroup<std::shared_ptr<UIComponent>> mUIComponents;

		size_t mWidth, mHeight, mFont;
	};
}