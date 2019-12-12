#include "TinyGLTFLoader.hpp"

#include "../../Scenes/Components/LightSources/PointLightSource.hpp"
#include "../../Scenes/Components/Materials/PhysicalBasedMaterial.hpp"
#include "../../Scenes/Components/Materials/WireframeMaterial.hpp"
#include "../../Scenes/Components/LinesMesh/CoordinateSystem.hpp"
#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../Scenes/Components/CollectionLabel.hpp"
#include "../../Scenes/Cameras/Camera.hpp"

#include "../../Shared/Textures/ConstantTexture.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <glm/gtx/matrix_decompose.hpp>
#include <tiny_gltf.h>

namespace LRTR {

	void TinyGLTFBuildScene(
		const std::shared_ptr<TinyGLTFScene>& tinyGLTFScene,
		const Matrix4x4f& transform,
		const tinygltf::Model* scene,
		const tinygltf::Node* node)
	{
		
	}
	
}

auto LRTR::TinyGLTFLoader::loadScene(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::string& sceneName,
	const std::string& fileName, 
	const Transform& transform)
	-> std::shared_ptr<TinyGLTFScene>
{
	tinygltf::Model model;
	std::string error;
	std::string warning;
	
	tinygltf::TinyGLTF loader;

	loader.LoadBinaryFromFile(&model, &error, &warning, fileName);
	
	auto tinyGLTFScene = std::make_shared<TinyGLTFScene>(sharing, sceneName, 2);

	std::vector<bool> isRoot(model.nodes.size(), true);

	for (size_t index = 0; index < model.nodes.size(); index++) {
		for (const auto& child : model.nodes[index].children) {
			isRoot[child] = false;
		}
	}

	for (size_t index = 0; index < model.nodes.size(); index++) {
		if (!isRoot[index]) continue;

		TinyGLTFBuildScene(tinyGLTFScene, transform.matrix(), &model, &model.nodes[index]);
	}

	if (fileName == "./Resources/Models/MetalRoughSpheresNoTextures.glb") {
		const auto light0 = std::make_shared<Shape>();
		const auto light1 = std::make_shared<Shape>();
		const auto light2 = std::make_shared<Shape>();

		light0->addComponent(std::make_shared<TransformWrap>(
			Vector3f(0.002f, 0.003f, 2.f),
			Vector4f(0, 0, 1, 0),
			Vector3f(1)));
		light0->addComponent(std::make_shared<PointLightSource>(Vector3f(10)));
		light0->component<CollectionLabel>()->set("Light", "light0");

		light1->addComponent(std::make_shared<TransformWrap>(
			Vector3f(0.002f, 5.f, 2.f),
			Vector4f(0, 0, 1, 0),
			Vector3f(1)));
		light1->addComponent(std::make_shared<PointLightSource>(Vector3f(10)));
		light1->component<CollectionLabel>()->set("Light", "light1");

		light2->addComponent(std::make_shared<TransformWrap>(
			Vector3f(5.f, 0.003f, 2.f),
			Vector4f(0, 0, 1, 0),
			Vector3f(1)));
		light2->addComponent(std::make_shared<PointLightSource>(Vector3f(10)));
		light2->component<CollectionLabel>()->set("Light", "light2");

		tinyGLTFScene->add(light0);
		tinyGLTFScene->add(light1);
		tinyGLTFScene->add(light2);
	}
	
	return tinyGLTFScene;
}
