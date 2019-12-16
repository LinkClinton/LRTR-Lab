#include "SceneManager.hpp"

#include "../UI/Components/SceneViewUIComponent.hpp"
#include "../UI/UIManager.hpp"

#include "../../../Extensions/TinyGLTF/TinyGLTFLoader.hpp"
#include "../../../Extensions/Assimp/AssimpLoader.hpp"

#include "../../../Scenes/Systems/PhysicalBasedRenderSystem.hpp"
#include "../../../Scenes/Systems/CollectionUpdateSystem.hpp"
#include "../../../Scenes/Systems/LinesMeshRenderSystem.hpp"
#include "../../../Scenes/Systems/WireframeRenderSystem.hpp"

#include "../../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../../Scenes/Components/CollectionLabel.hpp"
#include "../../../Scenes/Components/CameraGroup.hpp"
#include "../../../Scenes/Scene.hpp"

LRTR::SceneManager::SceneManager(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	Manager(sharing), mDevice(device)
{
	add(TinyGLTFLoader::loadScene(mRuntimeSharing, "Scene", "./Resources/Models/WaterBottle.glb"));

	const auto camera = std::make_shared<PerspectiveCamera>(
		std::make_shared<TransformWrap>(
			Vector3f(0, 0, 1),
			Vector4f(0, 0, 1, 0),
			Vector3f(1)),
		std::make_shared<Perspective>(
			MathUtility::pi<float>() * 0.125f,
			1920.0f,
			1080.0f,
			0.001f,
			1000.0f));

	camera->component<CollectionLabel>()->set("Collection", "Camera");
	
	mScenes["Scene"]->add(camera);

	mScenes["Scene"]->addSystem(std::make_shared<LinesMeshRenderSystem>(mRuntimeSharing, mDevice));
	//mScenes["Scene"]->addSystem(std::make_shared<WireframeRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<PhysicalBasedRenderSystem>(mRuntimeSharing, mDevice));
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

	const auto camera = mScenes["Scene"]->property()
		->component<CameraGroup>()->current();
	
	return mScenes["Scene"]->render(sceneTexture,
		camera == nullptr ? nullptr : 
		std::static_pointer_cast<SceneCamera>(camera),
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
