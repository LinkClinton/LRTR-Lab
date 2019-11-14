#include "UIManager.hpp"

#include "Components/SceneShapeUIComponent.hpp"
#include "Components/SceneViewUIComponent.hpp"
#include "Components/PropertyUIComponent.hpp"
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

	add("MainMenu", std::make_shared<MainMenuUIComponent>(mRuntimeSharing));

	add("View.Property", std::make_shared<PropertyUIComponent>(mRuntimeSharing));
	add("View.Logging", std::make_shared<LoggingUIComponent>(mRuntimeSharing));
	add("View.Scene", std::make_shared<SceneViewUIComponent>(mRuntimeSharing));
	add("View.Shape", std::make_shared<SceneShapeUIComponent>(mRuntimeSharing));
	
	//add("View.Manager", std::make_shared<ManagerUIComponent>(mRuntimeSharing));
	//add("Manager.SceneManager", std::make_shared<SceneManagerUIComponent>(mRuntimeSharing));
	//add("Manager.UIManager", std::make_shared<UIManagerUIComponent>(mRuntimeSharing));
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

void LRTR::UIManager::add(const std::string& name, const std::shared_ptr<UIComponent>& component)
{
	mUIComponents.insert({ name, component });

	mImGuiWindows->add(name, component->view());
}

void LRTR::UIManager::remove(const std::string& name)
{
	mUIComponents.erase(name);
	
	mImGuiWindows->remove(name);
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

auto LRTR::UIManager::components() const noexcept -> const StringOrderGroup<std::shared_ptr<UIComponent>>&
{
	return mUIComponents;
}