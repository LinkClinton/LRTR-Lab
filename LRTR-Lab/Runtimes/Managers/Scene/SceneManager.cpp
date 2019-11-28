#include "SceneManager.hpp"

#include "../UI/Components/SceneViewUIComponent.hpp"
#include "../UI/UIManager.hpp"

#include "../../../Extensions/Assimp/AssimpLoader.hpp"

#include "../../../Scenes/Components/TrianglesMesh/TrianglesMesh.hpp"
#include "../../../Scenes/Components/Materials/WireframeMaterial.hpp"
#include "../../../Scenes/Systems/CollectionUpdateSystem.hpp"
#include "../../../Scenes/Systems/LinesMeshRenderSystem.hpp"
#include "../../../Scenes/Systems/WireframeRenderSystem.hpp"
#include "../../../Scenes/Components/CameraGroup.hpp"
#include "../../../Scenes/Scene.hpp"

LRTR::SceneManager::SceneManager(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	Manager(sharing), mDevice(device)
{	
	add(AssimpLoader::loadScene(mRuntimeSharing, "Scene", "./Resources/Scenes/fishy_cat.glb"));

	mScenes["Scene"]->add("Camera", std::make_shared<PerspectiveCamera>(
		std::make_shared<TransformWrap>(
			Vector3f(19.4036f, -48.3112f, -2.55386f),
			Vector4f(0.964458f, 0.156321f, 0.213037f, glm::radians(104.1f)),
			Vector3f(2.18742f)),
		std::make_shared<Perspective>(
			MathUtility::pi<float>() * 0.125f,
			1920.0f,
			1080.0f)));
	
	mScenes["Scene"]->addSystem(std::make_shared<LinesMeshRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<WireframeRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<CollectionUpdateSystem>(mRuntimeSharing));
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
