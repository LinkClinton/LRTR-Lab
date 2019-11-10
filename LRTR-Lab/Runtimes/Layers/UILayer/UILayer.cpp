#include "UILayer.hpp"

#include "UIComponents/SceneViewUIComponent.hpp"
#include "UIComponents/MainMenuUIComponent.hpp"
#include "UIComponents/ConsoleUIComponent.hpp"
#include "UIComponents/LoggingUIComponent.hpp"

#include <CodeRed/Interface/GpuCommandQueue.hpp>

LRTR::UILayer::UILayer(
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const std::shared_ptr<CodeRed::GpuRenderPass>& renderPass,
	const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator,
	const std::shared_ptr<CodeRed::GpuCommandQueue>& queue,
	const size_t width, const size_t height) :
	mDevice(device), mRenderPass(renderPass),
	mCommandAllocator(allocator), mCommandQueue(queue),
	mWidth(width), mHeight(height)
{
	mCommandList = mDevice->createGraphicsCommandList(mCommandAllocator);

	ImGui::GetIO().Fonts->AddFontFromFileTTF("./Resources/Fonts/Consola.ttf", 20);

	mImGuiWindows = std::make_shared<CodeRed::ImGuiWindows>(
		mDevice, 
		mRenderPass,
		mCommandAllocator, 
		mCommandQueue, 2);

	auto layerSharing = std::make_shared<UILayerSharing>(this);
	
	mUIComponents.insert({ "MainMenu", std::make_shared<MainMenuUIComponent>(layerSharing) });
	mUIComponents.insert({ "View.Console", std::make_shared<ConsoleUIComponent>(layerSharing) });
	mUIComponents.insert({ "View.Logging", std::make_shared<LoggingUIComponent>(layerSharing) });
	mUIComponents.insert({ "View.Scene", std::make_shared<SceneViewUIComponent>(layerSharing) });

	for (const auto component : mUIComponents) mImGuiWindows->add(component.second->view());
}

void LRTR::UILayer::update(float delta)
{
	mImGuiWindows->update();
}

void LRTR::UILayer::render(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, float delta)
{
	mCommandList->beginRecording();
	mCommandList->beginRenderPass(mRenderPass, frameBuffer);

	mImGuiWindows->draw(mCommandList);

	mCommandList->endRenderPass();
	mCommandList->endRecording();

	mCommandQueue->execute({ mCommandList });
}

void LRTR::UILayer::resize(const size_t width, const size_t height)
{
	mWidth = width;
	mHeight = height;
}

auto LRTR::UILayer::width() const noexcept -> size_t
{
	return mWidth;
}

auto LRTR::UILayer::height() const noexcept -> size_t
{
	return mHeight;
}

auto LRTR::UILayer::components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>& 
{
	return mUIComponents;
}

auto LRTR::UILayer::device() const noexcept -> std::shared_ptr<CodeRed::GpuLogicalDevice>
{
	return mDevice;
}

auto LRTR::UILayer::commandAllocator() const noexcept -> std::shared_ptr<CodeRed::GpuCommandAllocator>
{
	return mCommandAllocator;
}

auto LRTR::UILayer::commandQueue() const noexcept -> std::shared_ptr<CodeRed::GpuCommandQueue>
{
	return mCommandQueue;
}

LRTR::UILayerSharing::UILayerSharing(UILayer* layer) : mLayer(layer)
{	
}

auto LRTR::UILayerSharing::components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>& 
{
	return mLayer->components();
}

auto LRTR::UILayerSharing::width() const noexcept -> size_t
{
	return mLayer->width();
}

auto LRTR::UILayerSharing::height() const noexcept -> size_t
{
	return mLayer->height();
}

auto LRTR::UILayerSharing::device() const noexcept -> std::shared_ptr<CodeRed::GpuLogicalDevice>
{
	return mLayer->device();
}

auto LRTR::UILayerSharing::commandAllocator() const noexcept -> std::shared_ptr<CodeRed::GpuCommandAllocator>
{
	return mLayer->commandAllocator();
}

auto LRTR::UILayerSharing::commandQueue() const noexcept -> std::shared_ptr<CodeRed::GpuCommandQueue>
{
	return mLayer->commandQueue();
}
