#include "SceneManager.hpp"

#include "../UI/Components/Managers/SceneManagerUIComponent.hpp"
#include "../UI/Components/SceneViewUIComponent.hpp"
#include "../UI/UIManager.hpp"

#include "../../../Scenes/Scene.hpp"

LRTR::SceneManager::SceneManager(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator) :
	Manager(sharing), mDevice(device), mCommandAllocator(allocator)
{
	mCommandList = mDevice->createGraphicsCommandList(mCommandAllocator);
	
	add(std::make_shared<Scene>("scene0", mRuntimeSharing->device()));
	add(std::make_shared<Scene>("scene1", mRuntimeSharing->device()));
	add(std::make_shared<Scene>("scene2", mRuntimeSharing->device()));
	add(std::make_shared<Scene>("scene3", mRuntimeSharing->device()));
	add(std::make_shared<Scene>("scene4", mRuntimeSharing->device()));
}

void LRTR::SceneManager::update(float delta)
{
}

auto LRTR::SceneManager::render(float delta) -> std::shared_ptr<CodeRed::GpuGraphicsCommandList>
{
	const auto sceneName = std::static_pointer_cast<SceneManagerUIComponent>(
		mRuntimeSharing->uiManager()->components().at("Manager.SceneManager"))->selected();

	const auto sceneTexture = std::static_pointer_cast<SceneViewUIComponent>(
		mRuntimeSharing->uiManager()->components().at("View.Scene"))->sceneTexture();

	if (sceneTexture == nullptr || mScenes.find(sceneName) == mScenes.end()) {
		mCommandList->beginRecording();

		mCommandList->endRecording();

		return mCommandList;
	}
	
	return mScenes[sceneName]->generate(sceneTexture, nullptr);
}

void LRTR::SceneManager::add(const std::shared_ptr<Scene>& scene)
{
	mScenes.insert({ scene->name(), scene });
}

void LRTR::SceneManager::remove(const std::string& name)
{
	mScenes.erase(name);
}

auto LRTR::SceneManager::scenes() const noexcept -> const StringGroup<std::shared_ptr<Scene>>& 
{
	return mScenes;
}
