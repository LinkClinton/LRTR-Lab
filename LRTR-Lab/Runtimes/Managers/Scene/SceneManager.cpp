#include "SceneManager.hpp"

#include "../UI/Components/SceneViewUIComponent.hpp"
#include "../UI/UIManager.hpp"

#include "../../../Scenes/Systems/CoordinateRenderSystem.hpp"
#include "../../../Scenes/Components/CameraGroup.hpp"
#include "../../../Scenes/Scene.hpp"

LRTR::SceneManager::SceneManager(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	Manager(sharing), mDevice(device)
{	
	add(std::make_shared<Scene>("Scene", mDevice));

	mScenes["Scene"]->add("Camera", std::make_shared<PerspectiveCamera>());

	mScenes["Scene"]->addSystem(std::make_shared<CoordinateRenderSystem>(mRuntimeSharing, mDevice));
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

	const auto camera = mScenes["Scene"]->shapes().at("Scene")
		->component<CameraGroup>()->current();
	
	return mScenes["Scene"]->render(sceneTexture,
		camera.empty() ? nullptr : 
		std::static_pointer_cast<SceneCamera>(mScenes["Scene"]->shapes().at(camera)),
		delta);
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
