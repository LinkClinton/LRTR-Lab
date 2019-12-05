#include "AssimpLoader.hpp"

#include "../../Scenes/Components/TrianglesMesh/TrianglesMesh.hpp"
#include "../../Scenes/Components/Materials/WireframeMaterial.hpp"
#include "../../Scenes/Components/LinesMesh/CoordinateSystem.hpp"
#include "../../Scenes/Components/CollectionLabel.hpp"
#include "../../Scenes/Cameras/Camera.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace LRTR {

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

			std::copy(
				reinterpret_cast<Vector3f*>(mesh->mVertices),
				reinterpret_cast<Vector3f*>(mesh->mVertices) + mesh->mNumVertices,
				vertices.data());

			totalNumIndices = 0;

			for (unsigned nFace = 0; nFace < mesh->mNumFaces; nFace++) {
				std::copy(
					mesh->mFaces[nFace].mIndices,
					mesh->mFaces[nFace].mIndices + mesh->mFaces[nFace].mNumIndices,
					indices.data() + totalNumIndices);

				totalNumIndices = totalNumIndices + mesh->mFaces[nFace].mNumIndices;
			}

			meshShape->component<CollectionLabel>()->set(node->mName.C_Str());
			
			meshShape->addComponent(std::make_shared<TransformWrap>(
				Vector3f(position.x, position.y, position.z),
				Vector4f(rAxis.x, rAxis.y, rAxis.z, rAngle),
				Vector3f(scale.x, scale.y, scale.z)));
			meshShape->addComponent(std::make_shared<WireframeMaterial>());
			meshShape->addComponent(std::make_shared<TrianglesMesh>(vertices, indices));

			assimpScene->add(mesh->mName.C_Str(), meshShape);
		}

		for (unsigned index = 0; index < node->mNumChildren; index++) 
			AssimpBuildScene(assimpScene, currentTransform, scene, node->mChildren[index]);
	}

}

auto LRTR::AssimpLoader::loadScene(
	const std::shared_ptr<RuntimeSharing>& sharing, 
	const std::string& sceneName,
	const std::string& fileName)
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
	
	AssimpBuildScene(assimpScene, aiMatrix4x4(), scene, scene->mRootNode);

	return assimpScene;
}
