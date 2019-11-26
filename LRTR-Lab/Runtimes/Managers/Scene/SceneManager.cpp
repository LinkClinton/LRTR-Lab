#include "SceneManager.hpp"

#include "../UI/Components/SceneViewUIComponent.hpp"
#include "../UI/UIManager.hpp"

#include "../../../Scenes/Components/TrianglesMesh/TrianglesMesh.hpp"
#include "../../../Scenes/Systems/LinesMeshRenderSystem.hpp"
#include "../../../Scenes/Components/CameraGroup.hpp"
#include "../../../Scenes/Scene.hpp"

LRTR::SceneManager::SceneManager(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	Manager(sharing), mDevice(device)
{	
	add(std::make_shared<Scene>("Scene", mDevice));

	mScenes["Scene"]->add("Camera", std::make_shared<PerspectiveCamera>());
	mScenes["Scene"]->add("TriangleMesh", std::make_shared<Shape>());
	
	mScenes["Scene"]->shapes().at("Camera")->component<TransformWrap>()
		->set(Vector3f(7, -6.92579f, 4.95831f), 
			Vector4f(0.773438f, 0.333831f, 0.538842f, glm::radians(77.3904f)),
			Vector3f(1));

	mScenes["Scene"]->shapes().at("TriangleMesh")->addComponent(
		std::make_shared<TrianglesMesh>());
	mScenes["Scene"]->shapes().at("TriangleMesh")->addComponent(
		std::make_shared<TransformWrap>());
	
	mScenes["Scene"]->addSystem(std::make_shared<LinesMeshRenderSystem>(mRuntimeSharing, mDevice));
}

void LRTR::SceneManager::update(float delta)
{
	mScenes["Scene"]->update(delta);
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
