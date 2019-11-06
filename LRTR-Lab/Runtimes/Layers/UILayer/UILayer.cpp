#include "UILayer.hpp"

#include "UIComponents/MainMenuUIComponent.hpp"
#include "UIComponents/ConsoleUIComponent.hpp"
#include "UIComponents/LoggingUIComponent.hpp"

#include <CodeRed/Interface/GpuCommandQueue.hpp>

LRTR::UILayer::UILayer(
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const std::shared_ptr<CodeRed::GpuRenderPass>& renderPass,
	const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator,
	const std::shared_ptr<CodeRed::GpuCommandQueue>& queue) :
	mDevice(device), mRenderPass(renderPass),
	mCommandAllocator(allocator), mCommandQueue(queue)
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

auto LRTR::UILayer::components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>& 
{
	return mUIComponents;
}

LRTR::UILayerSharing::UILayerSharing(UILayer* layer) : mLayer(layer)
{	
}

auto LRTR::UILayerSharing::components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>& 
{
	return mLayer->components();
}
