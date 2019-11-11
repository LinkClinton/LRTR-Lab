#include "UIManager.hpp"

#include "Components/SceneViewUIComponent.hpp"
#include "Components/MainMenuUIComponent.hpp"
#include "Components/ConsoleUIComponent.hpp"
#include "Components/LoggingUIComponent.hpp"

LRTR::UIManager::UIManager(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const std::shared_ptr<CodeRed::GpuRenderPass>& renderPass,
	const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator,
	const std::shared_ptr<CodeRed::GpuCommandQueue>& queue,
	const size_t width, const size_t height) : Manager(sharing),
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

	mUIComponents.insert({ "MainMenu", std::make_shared<MainMenuUIComponent>(mRuntimeSharing) });
	mUIComponents.insert({ "View.Console", std::make_shared<ConsoleUIComponent>(mRuntimeSharing) });
	mUIComponents.insert({ "View.Logging", std::make_shared<LoggingUIComponent>(mRuntimeSharing) });
	mUIComponents.insert({ "View.Scene", std::make_shared<SceneViewUIComponent>(mRuntimeSharing) });

	for (const auto component : mUIComponents) mImGuiWindows->add(component.second->view());
}

void LRTR::UIManager::update(float delta)
{
	mImGuiWindows->update();
}

auto LRTR::UIManager::render(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, float delta)
-> std::shared_ptr<CodeRed::GpuGraphicsCommandList>
{
	mCommandList->beginRecording();
	mCommandList->beginRenderPass(mRenderPass, frameBuffer);

	mImGuiWindows->draw(mCommandList);

	mCommandList->endRenderPass();
	mCommandList->endRecording();

	return mCommandList;
}

void LRTR::UIManager::resize(const size_t width, const size_t height)
{
	mWidth = width;
	mHeight = height;
}

auto LRTR::UIManager::width() const noexcept -> size_t
{
	return mWidth;
}

auto LRTR::UIManager::height() const noexcept -> size_t
{
	return mHeight;
}

auto LRTR::UIManager::components() const noexcept -> const StringGroup<std::shared_ptr<UIComponent>>&
{
	return mUIComponents;
}