#include "SceneManager.hpp"

#include "../UI/Components/SceneViewUIComponent.hpp"
#include "../UI/UIManager.hpp"

#include "../../../Extensions/TinyGLTF/TinyGLTFLoader.hpp"

#include "../../../Scenes/Systems/PhysicalBasedRenderSystem.hpp"
#include "../../../Scenes/Systems/MotionCameraUpdateSystem.hpp"
#include "../../../Scenes/Systems/PastEffectRenderSystem.hpp"
#include "../../../Scenes/Systems/CollectionUpdateSystem.hpp"
#include "../../../Scenes/Systems/LinesMeshRenderSystem.hpp"
#include "../../../Scenes/Systems/WireframeRenderSystem.hpp"

#include "../../../Scenes/Components/Materials/PhysicalBasedMaterial.hpp"
#include "../../../Scenes/Components/Materials/WireframeMaterial.hpp"
#include "../../../Scenes/Components/LightSources/PointLightSource.hpp"
#include "../../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../../Scenes/Components/MeshData/QuadMesh.hpp"
#include "../../../Scenes/Components/MeshData/BoxMesh.hpp"
#include "../../../Scenes/Components/Environment/SkyBox.hpp"
#include "../../../Scenes/Components/CollectionLabel.hpp"
#include "../../../Scenes/Components/CameraGroup.hpp"
#include "../../../Scenes/Cameras/MotionCamera.hpp"
#include "../../../Scenes/Scene.hpp"

#include "../../../Shared/Graphics/ResourceHelper.hpp"

#include "../../../Workflow/PBR/ImageBasedLightingWorkflow.hpp"

LRTR::SceneManager::SceneManager(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	Manager(sharing), mDevice(device)
{
	/*ImageBasedLightingWorkflow workflow(mDevice);

	auto input = ImageBasedLightingInput(mRuntimeSharing->queue(), mRuntimeSharing,
		"./Resources/Textures/HDR/newport_loft.hdr");
	
	auto output = workflow.start({ input });*/
	
	add(TinyGLTFLoader::loadScene(mRuntimeSharing, "Scene", "./Resources/Models/Sponza.glb",
		Transform::rotate(glm::pi<float>() * 0.5f, Vector3f(1, 0, 0)) *
		Transform::rotate(glm::pi<float>() * 1.5f, Vector3f(0, 1, 0))));

	const auto light = std::make_shared<Shape>();
	const auto light2 = std::make_shared<Shape>();
	
	light->addComponent(std::make_shared<PointLightSource>(Vector3f(50)));
	light->component<CollectionLabel>()->set("Light", "Point");
	light->addComponent(std::make_shared<TransformWrap>(
		Vector3f(0, 5.f, 2),
		Vector4f(1, 0, 0, 0),
		Vector3f(1)
		));

	light2->addComponent(std::make_shared<PointLightSource>(Vector3f(30)));
	light2->component<CollectionLabel>()->set("Light", "Point2");
	light2->addComponent(std::make_shared<TransformWrap>(
		Vector3f(0, 3.f, 3.f),
		Vector4f(1, 0, 0, 0),
		Vector3f(1)
		));
	
	mScenes["Scene"]->add(light);
	//mScenes["Scene"]->add(light2);

	const auto camera = std::make_shared<MotionCamera>(
		std::make_shared<TransformWrap>(
			Vector3f(0, 0, 1),
			Vector4f(1, 0, 0, glm::pi<float>() * 0.5f),
			Vector3f(1)),
		std::make_shared<Perspective>(
			MathUtility::pi<float>() * 0.25f,
			1920.0f,
			1080.0f,
			0.001f,
			1000.0f),
		std::make_shared<MotionProperty>());

	camera->component<CollectionLabel>()->set("Collection", "Camera");

	//mScenes["Scene"]->property()->addComponent(std::make_shared<SkyBox>(output.EnvironmentMap));
	
	/*mScenes["Scene"]->property()->addComponent(std::make_shared<SkyBox>(
		CodeRed::ResourceHelper::loadSkyBox(
			sharing->device(),
			sharing->allocator(),
			sharing->queue(),
			"./Resources/Textures/SkyBoxes/Sea"
		)));*/
	
	mScenes["Scene"]->add(camera);

	mScenes["Scene"]->addSystem(std::make_shared<LinesMeshRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<WireframeRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<PhysicalBasedRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<PastEffectRenderSystem>(mRuntimeSharing, device));

	mScenes["Scene"]->addSystem(std::make_shared<MotionCameraUpdateSystem>(mRuntimeSharing));
	mScenes["Scene"]->addSystem(std::make_shared<CollectionUpdateSystem>(mRuntimeSharing));

	/*for (auto& system : mScenes["Scene"]->systems()) {
		if (std::dynamic_pointer_cast<PhysicalBasedRenderSystem>(system) != nullptr) {
			auto pbrSystem = std::dynamic_pointer_cast<PhysicalBasedRenderSystem>(system);
			
			pbrSystem->setEnvironmentLight({
				output.IrradianceMap,
				output.PreFilteringMap,
				output.PreComputingBRDF
			});
		}
	}*/
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
