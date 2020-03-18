#include "SceneManager.hpp"

#include "../UI/Components/SceneViewUIComponent.hpp"
#include "../UI/UIManager.hpp"

#include "../../../Extensions/TinyGLTF/TinyGLTFLoader.hpp"

#include "../../../Scenes/Systems/PhysicalBasedRenderSystem.hpp"
#include "../../../Scenes/Systems/MotionCameraUpdateSystem.hpp"
#include "../../../Scenes/Systems/PostEffectRenderSystem.hpp"
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
	//auto workflow = std::make_shared<ImageBasedLightingWorkflow>(mDevice);

	/*const auto output = workflow->start({ ImageBasedLightingInput(
		mRuntimeSharing->queue(),
		mRuntimeSharing,
		"./Resources/Textures/HDR/newport_loft.hdr"
	) });*/
	
	add(std::make_shared<Scene>("Scene", mDevice));

	//add(TinyGLTFLoader::loadScene(mRuntimeSharing, "Scene", "./Resources/Models/WaterBottle.glb", Transform::translate(Vector3f(0, -2, 0))));
	
	const auto light0 = std::make_shared<Shape>();
	const auto light1 = std::make_shared<Shape>();
	const auto light2 = std::make_shared<Shape>();

	const auto quad = std::make_shared<Shape>();
	const auto box0 = std::make_shared<Shape>();
	const auto box1 = std::make_shared<Shape>();
	const auto box2 = std::make_shared<Shape>();
	const auto box3 = std::make_shared<Shape>();
	
	quad->addComponent<TrianglesMesh>(std::make_shared<QuadMesh>(20.f, 20.f));
	quad->addComponent(std::make_shared<TransformWrap>());
	quad->addComponent(std::make_shared<PhysicalBasedMaterial>(
		Vector4f(0), Vector4f(1), Vector4f(0.7f), Vector4f(0)
		));
	quad->component<CollectionLabel>()->set("Objects", "Quad");

	box0->addComponent<TrianglesMesh>(std::make_shared<BoxMesh>(1.f, 1.f, 1.f));
	box0->addComponent(std::make_shared<TransformWrap>(
		Vector3f(1, 0, 0.5f), Vector4f(), Vector3f(1)
		));
	box0->addComponent(std::make_shared<PhysicalBasedMaterial>(
		Vector4f(0.f), Vector4f(0.7f, 0.6f, 0.65f, 1.0f), Vector4f(0.5f), Vector4f(0)
		));
	box0->component<CollectionLabel>()->set("Objects", "Box0");

	box1->addComponent<TrianglesMesh>(std::make_shared<BoxMesh>(1.f, 1.f, 1.f));
	box1->addComponent(std::make_shared<TransformWrap>(
		Vector3f(-1, 0, 0.5f), Vector4f(), Vector3f(1)
		));
	box1->addComponent(std::make_shared<PhysicalBasedMaterial>(
		Vector4f(0.33f), Vector4f(0.7f, 0.6f, 0.65f, 1.0f), Vector4f(0.6f), Vector4f(0)
		));
	box1->component<CollectionLabel>()->set("Objects", "Box1");

	box2->addComponent<TrianglesMesh>(std::make_shared<BoxMesh>(1.f, 1.f, 1.f));
	box2->addComponent(std::make_shared<TransformWrap>(
		Vector3f(0, 1, 0.5f), Vector4f(), Vector3f(1)
		));
	box2->addComponent(std::make_shared<PhysicalBasedMaterial>(
		Vector4f(0.66f), Vector4f(0.7f, 0.6f, 0.65f, 1.0f), Vector4f(0.7f), Vector4f(0)
		));
	box2->component<CollectionLabel>()->set("Objects", "Box2");

	box3->addComponent<TrianglesMesh>(std::make_shared<BoxMesh>(1.f, 1.f, 1.f));
	box3->addComponent(std::make_shared<TransformWrap>(
		Vector3f(0, -1, 0.5f), Vector4f(), Vector3f(1)
		));
	box3->addComponent(std::make_shared<PhysicalBasedMaterial>(
		Vector4f(1.f), Vector4f(0.7f, 0.6f, 0.65f, 1.0f), Vector4f(0.8f), Vector4f(0)
		));
	box3->component<CollectionLabel>()->set("Objects", "Box3");

	//box0->component<PhysicalBasedMaterial>()->IsBlurred = true;
	//box1->component<PhysicalBasedMaterial>()->IsBlurred = true;
	//box2->component<PhysicalBasedMaterial>()->IsBlurred = true;
	//box3->component<PhysicalBasedMaterial>()->IsBlurred = true;
	
	light0->addComponent(std::make_shared<PointLightSource>(Vector3f(20)));
	light0->component<CollectionLabel>()->set("Light", "Point0");
	light0->addComponent(std::make_shared<TransformWrap>(
		Vector3f(0, 5.f, 3.f),
		Vector4f(1, 0, 0, 0),
		Vector3f(1)
		));

	light1->addComponent(std::make_shared<PointLightSource>(Vector3f(20)));
	light1->component<CollectionLabel>()->set("Light", "Point1");
	light1->addComponent(std::make_shared<TransformWrap>(
		Vector3f(5.f, 0.f, 3.f),
		Vector4f(1, 0, 0, 0),
		Vector3f(1)
		));

	light2->addComponent(std::make_shared<PointLightSource>(Vector3f(20)));
	light2->component<CollectionLabel>()->set("Light", "Point2");
	light2->addComponent(std::make_shared<TransformWrap>(
		Vector3f(0.f, 0.f, 3.5f),
		Vector4f(1, 0, 0, 0),
		Vector3f(1)
		));

	mScenes["Scene"]->add(quad);
	mScenes["Scene"]->add(box0);
	mScenes["Scene"]->add(box1);
	mScenes["Scene"]->add(box2);
	mScenes["Scene"]->add(box3);
	
	mScenes["Scene"]->add(light0);
	mScenes["Scene"]->add(light1);
	mScenes["Scene"]->add(light2);
	
	const auto camera = std::make_shared<MotionCamera>(
		std::make_shared<TransformWrap>(
			Vector3f(0.081f, 1.995f, 5.649f),
			QuaternionF(0.029f, 0.006f, 0.209f, 0.978f),
			Vector3f(1)),
		std::make_shared<Perspective>(
			MathUtility::pi<float>() * 0.25f,
			1920.0f,
			1080.0f,
			0.001f,
			1000.0f),
		std::make_shared<MotionProperty>(0.01f, 3.f, 
			std::array<bool, 3>{ true, true, false }));

	camera->component<CollectionLabel>()->set("Collection", "Camera");

	//mScenes["Scene"]->property()->addComponent(std::make_shared<SkyBox>(output.EnvironmentMap));
	
	mScenes["Scene"]->property()->addComponent(std::make_shared<SkyBox>(
		CodeRed::ResourceHelper::loadSkyBox(
			sharing->device(),
			sharing->allocator(),
			sharing->queue(),
			"./Resources/Textures/SkyBoxes/Sea"
		)));
	
	mScenes["Scene"]->add(camera);

	mScenes["Scene"]->addSystem(std::make_shared<LinesMeshRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<WireframeRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<PhysicalBasedRenderSystem>(mRuntimeSharing, mDevice));
	mScenes["Scene"]->addSystem(std::make_shared<PostEffectRenderSystem>(mRuntimeSharing, device));

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
