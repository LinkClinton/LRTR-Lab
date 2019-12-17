#include "TinyGLTFLoader.hpp"

#include "../../Scenes/Components/LightSources/PointLightSource.hpp"
#include "../../Scenes/Components/Materials/PhysicalBasedMaterial.hpp"
#include "../../Scenes/Components/Materials/WireframeMaterial.hpp"
#include "../../Scenes/Components/LinesMesh/CoordinateSystem.hpp"
#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../Scenes/Components/CollectionLabel.hpp"
#include "../../Scenes/Cameras/Camera.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Textures/ConstantTexture.hpp"
#include "../../Shared/Textures/ImageTexture.hpp"

#define TINY_GLTF_HAS_VALUE(value) (value >= 0)
#define TINY_GLTF_TRY_READ_MATERIAL_VALUE(texture, value) \
	if (material->values.find(value) != material->values.end()) \
		texture = readMaterialValue(sharing, material->values.at(value), scene);

#define TINY_GLTF_TRY_READ_MATERIAL_ADDITIONAL_VALUE(texture, value) \
	if (material->additionalValues.find(value) != material->additionalValues.end()) \
		texture = readMaterialValue(sharing, material->additionalValues.at(value), scene);

#define LRTR_TRY_EXECUTE(condition, expression) if (condition) expression;


#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <glm/gtx/matrix_decompose.hpp>
#include <tiny_gltf.h>

namespace LRTR {
	
	template<typename T>
	void readAccessor(std::vector<T>& data, const tinygltf::Accessor* accessor, const tinygltf::Model* scene) {
		const auto& bufferView = scene->bufferViews[accessor->bufferView];
		const auto& buffer = scene->buffers[bufferView.buffer];

		data = std::vector<T>(accessor->count);

		std::memcpy(data.data(), buffer.data.data() + bufferView.byteOffset + accessor->byteOffset,
			accessor->count * sizeof(T));
	}

	void readUnsignedAccessor(std::vector<unsigned>& data, const tinygltf::Accessor* accessor, const tinygltf::Model* scene) {
		const auto& bufferView = scene->bufferViews[accessor->bufferView];
		const auto& buffer = scene->buffers[bufferView.buffer];

		if (accessor->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) { readAccessor(data, accessor, scene); return; }
		
		data = std::vector<unsigned>(accessor->count);

		if (accessor->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
			std::vector<unsigned short> tempData;

			readAccessor(tempData, accessor, scene);

			for (size_t index = 0; index < tempData.size(); index++)
				data[index] = tempData[index];
		}
	}
	
	void readVector3fAccessor(std::vector<Vector3f>& data, const tinygltf::Accessor* accessor, const tinygltf::Model* scene) {
		const auto& bufferView = scene->bufferViews[accessor->bufferView];
		const auto& buffer = scene->buffers[bufferView.buffer];

		assert(accessor->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

		if (accessor->type == 3) { readAccessor(data, accessor, scene); return; }

		data = std::vector<Vector3f>(accessor->count);
		
		if (accessor->type == 2) {
			std::vector<Vector2f> tempData;
			
			readAccessor(tempData, accessor, scene);

			for (size_t index = 0; index < tempData.size(); index++) 
				data[index] = Vector3f(tempData[index], 0);
		}

		if (accessor->type == 4) {
			std::vector<Vector4f> tempData;

			readAccessor(tempData, accessor, scene);

			for (size_t index = 0; index < tempData.size(); index++)
				data[index] = Vector3f(tempData[index]);
		}
	}

	auto readMaterialValue(
		const std::shared_ptr<RuntimeSharing>& sharing,
		const tinygltf::Parameter& parameter,
		const tinygltf::Model* scene) -> std::shared_ptr<Texture> {
		if (parameter.has_number_value) return std::make_shared<ConstantTexture<Vector1f>>(Vector1f(
			static_cast<float>(parameter.number_value)));

		if (!parameter.number_array.empty() && parameter.number_array.size() == 4)
			return std::make_shared<ConstantTexture<Vector4f>>(Vector4f(
				static_cast<float>(parameter.number_array[0]),
				static_cast<float>(parameter.number_array[1]),
				static_cast<float>(parameter.number_array[2]),
				static_cast<float>(parameter.number_array[3])));

		if (TINY_GLTF_HAS_VALUE(parameter.TextureIndex())) {
			const auto& texture = scene->textures[parameter.TextureIndex()];
			const auto& image = scene->images[texture.source];

			const auto gpuTexture = sharing->device()->createTexture(
				CodeRed::ResourceInfo::Texture2D(
					image.width, image.height,
					CodeRed::PixelFormat::RedGreenBlueAlpha8BitUnknown
				)
			);

			CodeRed::ResourceHelper::updateTexture(sharing->device(), sharing->allocator(),
				sharing->queue(), gpuTexture, image.image.data());

			return std::make_shared<ImageTexture>(gpuTexture);
		}

		return nullptr;
	}
	
	auto readMaterialFactorValue(
		const std::shared_ptr<RuntimeSharing>& sharing,
		const tinygltf::ParameterMap& mapped,
		const tinygltf::Model* scene,
		const std::string& name) -> std::shared_ptr<ConstantTexture4F>
	{
		if (mapped.find(name) == mapped.end()) return nullptr;

		return std::static_pointer_cast<ConstantTexture4F>(
			readMaterialValue(sharing, mapped.at(name), scene));
	}

	auto readMaterialTextureValue(
		const std::shared_ptr<RuntimeSharing>& sharing,
		const tinygltf::ParameterMap& mapped,
		const tinygltf::Model* scene,
		const std::string& name) -> std::shared_ptr<ImageTexture>
	{
		if (mapped.find(name) == mapped.end()) return nullptr;

		return std::static_pointer_cast<ImageTexture>(
			readMaterialValue(sharing, mapped.at(name), scene));
	}
	
	auto readMaterial(
		const std::shared_ptr<RuntimeSharing>& sharing,
		const tinygltf::Material* material, 
		const tinygltf::Model* scene) -> std::shared_ptr<PhysicalBasedMaterial>
	{
		auto metallicFactor = readMaterialFactorValue(sharing, material->values, scene, "metallicFactor");
		auto baseColorFactor = readMaterialFactorValue(sharing, material->values, scene, "baseColorFactor");
		auto roughnessFactor = readMaterialFactorValue(sharing, material->values, scene, "roughnessFactor");

		auto metallicRoughnessTexture = readMaterialTextureValue(sharing, material->values, scene, "metallicRoughnessTexture");
		auto baseColorTexture = readMaterialTextureValue(sharing, material->values, scene, "baseColorTexture"); 
		auto occlusionTexture = readMaterialTextureValue(sharing, material->additionalValues, scene, "occlusionTexture");
		auto normalMapTexture = readMaterialTextureValue(sharing, material->additionalValues, scene, "normalTexture");

		return std::make_shared<PhysicalBasedMaterial>(
			metallicFactor, baseColorFactor, roughnessFactor,
			metallicRoughnessTexture, baseColorTexture, metallicRoughnessTexture,
			occlusionTexture, normalMapTexture);
	}
	
	void TinyGLTFBuildScene(
		const std::shared_ptr<RuntimeSharing>& sharing,
		const std::shared_ptr<TinyGLTFScene>& tinyGLTFScene,
		const Matrix4x4f& transform,
		const tinygltf::Model* scene,
		const tinygltf::Node* node)
	{
		auto translation = node->translation.empty() ? Vector3f() :
			Vector3f(
				static_cast<float>(node->translation[0]),
				static_cast<float>(node->translation[1]), 
				static_cast<float>(node->translation[2]));

		auto rotation = node->rotation.empty() ? QuaternionF() :
			QuaternionF(
				static_cast<float>(node->rotation[0]),
				static_cast<float>(node->rotation[1]),
				static_cast<float>(node->rotation[2]),
				static_cast<float>(node->rotation[3]));

		auto scale = node->scale.empty() ? Vector3f(1) :
			Vector3f(
				static_cast<float>(node->scale[0]), 
				static_cast<float>(node->scale[2]),
				static_cast<float>(node->scale[2]));

		const auto angle = glm::angle(rotation);
		const auto axis = glm::axis(rotation);
		
		const auto currentTransform = Transform(translation, glm::angleAxis(angle, 
			Vector3f(axis.z, axis.x, axis.y)), scale).matrix() * transform;
		
		MathUtility::decompose(currentTransform, translation, rotation, scale);
		
		if (TINY_GLTF_HAS_VALUE(node->mesh)) {
			const auto& mesh = scene->meshes[node->mesh];

			for (size_t index = 0; index < mesh.primitives.size(); index++) {
				const auto& primitives = mesh.primitives[index];
				const auto meshShape = std::make_shared<Shape>();

				std::vector<Vector3f> positions;
				std::vector<Vector3f> texCoords;
				std::vector<Vector3f> tangents;
				std::vector<Vector3f> normals;

				std::vector<unsigned> indices;

				LRTR_TRY_EXECUTE(
					primitives.attributes.find("TEXCOORD_0") != primitives.attributes.end(),
					readVector3fAccessor(texCoords, &scene->accessors[primitives.attributes.at("TEXCOORD_0")], scene)
				);

				LRTR_TRY_EXECUTE(
					primitives.attributes.find("TANGENT") != primitives.attributes.end(),
					readVector3fAccessor(tangents, &scene->accessors[primitives.attributes.at("TANGENT")], scene)
				);

				LRTR_TRY_EXECUTE(
					primitives.attributes.find("NORMAL") != primitives.attributes.end(),
					readVector3fAccessor(normals, &scene->accessors[primitives.attributes.at("NORMAL")], scene)
				);

				LRTR_TRY_EXECUTE(
					primitives.attributes.find("POSITION") != primitives.attributes.end(),
					readVector3fAccessor(positions, &scene->accessors[primitives.attributes.at("POSITION")], scene)
				);

				readUnsignedAccessor(indices, &scene->accessors[primitives.indices], scene);

				meshShape->component<CollectionLabel>()->set(node->name, mesh.name + std::to_string(index));

				meshShape->addComponent(std::make_shared<TransformWrap>(
					translation, rotation, scale));
				//meshShape->addComponent(std::make_shared<WireframeMaterial>());
				meshShape->addComponent(std::make_shared<TrianglesMesh>(
					positions, texCoords, tangents, normals, indices));
				meshShape->addComponent(
					TINY_GLTF_HAS_VALUE(primitives.material) ? readMaterial(
						sharing, &scene->materials[primitives.material], scene) :
					std::make_shared<PhysicalBasedMaterial>());

				tinyGLTFScene->add(meshShape);
			}
		}

		for (const auto& child : node->children) {
			TinyGLTFBuildScene(sharing, tinyGLTFScene, currentTransform, scene, &scene->nodes[child]);
		}
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

		TinyGLTFBuildScene(sharing, tinyGLTFScene, transform.matrix(), &model, &model.nodes[index]);
	}

	if (fileName == "./Resources/Models/WaterBottle.glb") {
		const auto light0 = std::make_shared<Shape>();
		const auto light1 = std::make_shared<Shape>();
		const auto light2 = std::make_shared<Shape>();

		light0->addComponent(std::make_shared<TransformWrap>(
			Vector3f(0.f, 0.5f, 1.f),
			Vector4f(0, 0, 1, 0),
			Vector3f(1)));
		light0->addComponent(std::make_shared<PointLightSource>(Vector3f(5)));
		light0->component<CollectionLabel>()->set("Light", "light0");

		light1->addComponent(std::make_shared<TransformWrap>(
			Vector3f(1.f, 0.5f, 0.f),
			Vector4f(0, 0, 1, 0),
			Vector3f(1)));
		light1->addComponent(std::make_shared<PointLightSource>(Vector3f(5)));
		light1->component<CollectionLabel>()->set("Light", "light1");

		light2->addComponent(std::make_shared<TransformWrap>(
			Vector3f(-1.f, 0.5f, 0.f),
			Vector4f(0, 0, 1, 0),
			Vector3f(1)));
		light2->addComponent(std::make_shared<PointLightSource>(Vector3f(5)));
		light2->component<CollectionLabel>()->set("Light", "light2");

		tinyGLTFScene->add(light0);
		tinyGLTFScene->add(light1);
		tinyGLTFScene->add(light2);
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
