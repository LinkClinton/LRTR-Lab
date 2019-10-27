#include "UILayer.hpp"

#include "UIComponents/MainMenuUIComponent.hpp"

#include <CodeRed\Interface\GpuCommandQueue.hpp>

LRTR::UILayer::UILayer(
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const std::shared_ptr<CodeRed::GpuRenderPass>& renderPass,
	const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator,
	const std::shared_ptr<CodeRed::GpuCommandQueue>& queue) :
	mDevice(device), mRenderPass(renderPass),
	mCommandAllocator(allocator), mCommandQueue(queue)
{
	mCommandList = mDevice->createGraphicsCommandList(mCommandAllocator);

	ImGui::GetIO().FontGlobalScale = 1.5f;

	mImGuiWindows = std::make_shared<CodeRed::ImGuiWindows>(
		mDevice, 
		mRenderPass,
		mCommandAllocator, 
		mCommandQueue, 2);

	mUIComponents.insert({ "Main Menu", std::make_shared<MainMenuUIComponent>() });

	for (auto component : mUIComponents) mImGuiWindows->add(component.second->view());
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
