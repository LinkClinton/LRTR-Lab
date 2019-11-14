#include "SceneManager.hpp"

#include "../UI/Components/Managers/SceneManagerUIComponent.hpp"
#include "../UI/Components/SceneViewUIComponent.hpp"
#include "../UI/UIManager.hpp"

#include "../../../Scenes/Scene.hpp"

LRTR::SceneManager::SceneManager(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	Manager(sharing), mDevice(device)
{	
	add(std::make_shared<Scene>("Scene", mDevice));

	mScenes["Scene"]->add("Shape0", std::make_shared<Shape>());
	mScenes["Scene"]->add("Shape1", std::make_shared<Shape>());
	mScenes["Scene"]->add("Shape2", std::make_shared<Shape>());
	mScenes["Scene"]->add("Shape3", std::make_shared<Shape>());

	mScenes["Scene"]->shapes().at("Shape0")->addComponent(
		std::make_shared<Transform>());
}

void LRTR::SceneManager::update(float delta)
{
}

auto LRTR::SceneManager::render(float delta) ->
	std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>
{
	const auto sceneTexture = std::static_pointer_cast<SceneViewUIComponent>(
		mRuntimeSharing->uiManager()->components().at("View.Scene"))->sceneTexture();

	if (sceneTexture == nullptr) return {};
	
	return mScenes["Scene"]->generate(sceneTexture, nullptr);
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
