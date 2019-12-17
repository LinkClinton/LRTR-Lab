#include "AssimpLoader.hpp"

#include "../../Scenes/Components/LightSources/PointLightSource.hpp"
#include "../../Scenes/Components/Materials/PhysicalBasedMaterial.hpp"
#include "../../Scenes/Components/Materials/WireframeMaterial.hpp"
#include "../../Scenes/Components/LinesMesh/CoordinateSystem.hpp"
#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../Scenes/Components/CollectionLabel.hpp"
#include "../../Scenes/Cameras/Camera.hpp"

#include "../../Shared/Textures/ConstantTexture.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#define LRTR_TRY_EXECUTE(condition, expression) if (condition) expression;

namespace LRTR {

	void copyVertexAttribute(Vector3f* dest, aiVector3D* src, size_t size) {
		std::copy(
			reinterpret_cast<Vector3f*>(src),
			reinterpret_cast<Vector3f*>(src) + size,
			dest
		);
	}

	void copyVertexAttribute(std::vector<Vector3f>& dest, aiVector3D* src, size_t size) {
		dest = std::vector<Vector3f>(size);

		copyVertexAttribute(dest.data(), src, size);
	}

	auto readVector4f(aiMaterialProperty* property) -> Vector4f {
		assert(property->mDataLength == 16 && property->mType == aiPTI_Float);

		Vector4f data;

		std::memcpy(&data, property->mData, property->mDataLength);

		return data;
	}

	auto readVector1f(aiMaterialProperty* property) -> Vector4f {
		assert(property->mDataLength == 4 && property->mType == aiPTI_Float);

		Vector1f data;

		std::memcpy(&data, property->mData, property->mDataLength);

		return Vector4f(data);
	}

	auto convertTo(aiMaterial* material) -> std::shared_ptr<PhysicalBasedMaterial> {
		std::shared_ptr<ConstantTexture4F> roughness;
		std::shared_ptr<ConstantTexture4F> baseColor;
		std::shared_ptr<ConstantTexture4F> metallic;
		
		for (unsigned index = 0; index < material->mNumProperties; index++) {
			const auto& property = material->mProperties[index];
			const auto key = std::string(property->mKey.C_Str());

			if (key.find("pbrMetallicRoughness") == std::string::npos) continue;
			
			//this property is roughness
			if (key.find("roughnessFactor") != std::string::npos) {
				//non-texture property, we will read 1 floats
				if (property->mType == aiPTI_Float && property->mSemantic == 0) {
					roughness = std::make_shared<ConstantTexture<Vector4f>>(readVector1f(property));
				}
			}
			
			//this property is base color
			if (key.find("baseColorFactor") != std::string::npos) {
				//non-texture property, we will read 4 floats
				if (property->mType == aiPTI_Float && property->mSemantic == 0) {
					baseColor = std::make_shared<ConstantTexture<Vector4f>>(readVector4f(property));
				}
			}

			//this property is metallic
			if (key.find("metallicFactor") != std::string::npos) {
				//non-texture property, we will read 1 floats
				if (property->mType == aiPTI_Float && property->mSemantic == 0) {
					metallic = std::make_shared<ConstantTexture<Vector4f>>(readVector1f(property));
				}
			}
		}
		
		return std::make_shared<PhysicalBasedMaterial>(metallic, baseColor, roughness);
	}
	
	void AssimpBuildScene(
		const std::shared_ptr<AssimpScene>& assimpScene,
		const aiMatrix4x4& transform,
		const aiScene* scene,
		const aiNode* node)
	{
		const auto currentTransform = node->mTransformation * transform;

		aiVector3D position;
		aiVector3D rAxis;
		aiVector3D scale;
		float rAngle;
		
		currentTransform.Decompose(scale, rAxis, rAngle, position);

		for (unsigned index = 0; index < node->mNumMeshes; index++) {
			const auto mesh = scene->mMeshes[node->mMeshes[index]];
			const auto meshShape = std::make_shared<Shape>();

			size_t totalNumIndices = 0;

			for (unsigned nFace = 0; nFace < mesh->mNumFaces; nFace++)
				totalNumIndices = totalNumIndices + mesh->mFaces[nFace].mNumIndices;

			std::vector<Vector3f> vertices(mesh->mNumVertices);
			std::vector<unsigned> indices(totalNumIndices);

			std::vector<Vector3f> texCoords;
			std::vector<Vector3f> tangents;
			std::vector<Vector3f> normals;
			
			copyVertexAttribute(vertices.data(), mesh->mVertices, mesh->mNumVertices);

			LRTR_TRY_EXECUTE(
				mesh->HasTextureCoords(0),
				copyVertexAttribute(texCoords, mesh->mTextureCoords[0], mesh->mNumVertices)
			);

			LRTR_TRY_EXECUTE(
				mesh->HasTangentsAndBitangents(),
				copyVertexAttribute(tangents, mesh->mTangents, mesh->mNumVertices);
			);

			LRTR_TRY_EXECUTE(
				mesh->HasNormals(),
				copyVertexAttribute(normals, mesh->mNormals, mesh->mNumVertices)
			);

			totalNumIndices = 0;
			
			for (unsigned nFace = 0; nFace < mesh->mNumFaces; nFace++) {
				std::copy(
					mesh->mFaces[nFace].mIndices,
					mesh->mFaces[nFace].mIndices + mesh->mFaces[nFace].mNumIndices,
					indices.data() + totalNumIndices);

				totalNumIndices = totalNumIndices + mesh->mFaces[nFace].mNumIndices;
			}

			meshShape->component<CollectionLabel>()->set(node->mName.C_Str(), mesh->mName.C_Str());
			
			meshShape->addComponent(std::make_shared<TransformWrap>(
				Vector3f(position.x, position.y, position.z),
				Vector4f(rAxis.x, rAxis.y, rAxis.z, rAngle),
				Vector3f(scale.x, scale.y, scale.z)));
			//meshShape->addComponent(std::make_shared<WireframeMaterial>());
			meshShape->addComponent(std::make_shared<TrianglesMesh>(
				vertices, texCoords, tangents, normals, indices));
			meshShape->addComponent(convertTo(scene->mMaterials[mesh->mMaterialIndex]));

			assimpScene->add(meshShape);
		}

		for (unsigned index = 0; index < node->mNumChildren; index++) 
			AssimpBuildScene(assimpScene, currentTransform, scene, node->mChildren[index]);
	}

}

auto LRTR::AssimpLoader::loadScene(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::string& sceneName,
	const std::string& fileName,
	const Transform& transform)
	-> std::shared_ptr<AssimpScene>
{
	Assimp::Importer importer;
	
	const auto scene = importer.ReadFile(fileName,
		aiProcess_Triangulate |
		aiProcess_EmbedTextures);
	
	auto assimpScene = std::make_shared<AssimpScene>(sharing, sceneName, 2);

	LRTR_DEBUG_WARNING_IF(!std::string(importer.GetErrorString()).empty(),
		importer.GetErrorString());

	if (scene == nullptr) return assimpScene;

	const auto matrix = transform.matrix();
	
	AssimpBuildScene(assimpScene, aiMatrix4x4(
		matrix[0].x, matrix[0].y, matrix[0].z, matrix[0].w,
		matrix[1].x, matrix[1].y, matrix[1].z, matrix[1].w,
		matrix[2].x, matrix[2].y, matrix[2].z, matrix[2].w,
		matrix[3].x, matrix[3].y, matrix[3].z, matrix[3].w
	), scene, scene->mRootNode);

	//temp
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

		assimpScene->add(light0);
		assimpScene->add(light1);
		assimpScene->add(light2);
	}
	
	return assimpScene;
}
