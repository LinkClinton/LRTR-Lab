#include "PhysicalBasedRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Scenes/Components/Materials/PhysicalBasedMaterial.hpp"
#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../Shared/Textures/ConstantTexture.hpp"
#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"

#define LRTR_INSERT_VERTEX_PROPERTY(condition, dest, source0, source1) \
	if (condition) dest.insert(dest.end(), source0.begin(), source0.end()); \
	else dest.insert(dest.end(), source1.begin(), source1.end());

#define LRTR_IS_CONSTANT_TEXTURE(texture) \
	(std::dynamic_pointer_cast<ConstantTexture<Vector4f>>(texture) != nullptr)

#define LRTR_RESET_AND_COPY_BUFFER(buffer, name) \
	if (buffer != mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)) { \
		CodeRed::ResourceHelper::copyBuffer(buffer, mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)); \
		mFrameResources[mCurrentFrameIndex].set(name, buffer); \
	}

#define LRTR_RESET_BUFFER(buffer, name) \
	if (buffer != mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)) { \
		mFrameResources[mCurrentFrameIndex].set(name, buffer); \
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap")->bindBuffer(buffer, 0); \
	}

namespace LRTR {
	
	struct SharedMaterial {
		Vector4f BaseColor;
		Vector4f Roughness;
		Vector4f Metallic;
	};
	
}

LRTR::PhysicalBasedRenderSystem::PhysicalBasedRenderSystem(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device, 
	size_t maxFrameCount) : RenderSystem(sharing, device, maxFrameCount)
{
	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(
			sizeof(Matrix4x4f)
		)
	);

	//resource 0 : material properties
	//resource 1 : transform matrix
	//resource 2 : camera matrix
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer,1),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer,2)
		}, {}, CodeRed::Constant32Bits(6, 8));

	for (auto& frameResource : mFrameResources) {
		auto vertexBuffers = std::make_shared<std::vector<std::shared_ptr<CodeRed::GpuBuffer>>>(4);
		auto descriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);

		//vertex buffer 0 : position property
		//vertex buffer 1 : texcoord property
		//vertex buffer 2 : tangent property
		//vertex buffer 3 : normal property
		for (size_t index = 0; index < vertexBuffers->size(); index++) {
			(*vertexBuffers)[index] = mDevice->createBuffer(
				CodeRed::ResourceInfo::VertexBuffer(
					sizeof(Vector3f),
					1000,
					CodeRed::MemoryHeap::Upload
				)
			);
		}
		
		auto indexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::IndexBuffer(
				sizeof(unsigned),
				3000,
				CodeRed::MemoryHeap::Upload
			)
		);

		auto transformBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(Matrix4x4f),
				100,
				CodeRed::MemoryHeap::Upload
			)
		);

		auto materialBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(SharedMaterial),
				100,
				CodeRed::MemoryHeap::Upload
			)
		);

		descriptorHeap->bindBuffer(materialBuffer, 0);
		descriptorHeap->bindBuffer(transformBuffer, 1);
		descriptorHeap->bindBuffer(mViewBuffer, 2);

		frameResource.set("DescriptorHeap", descriptorHeap);
		frameResource.set("VertexBuffer", vertexBuffers);
		frameResource.set("IndexBuffer", indexBuffer);
		frameResource.set("TransformBuffer", transformBuffer);
		frameResource.set("MaterialBuffer", materialBuffer);
		frameResource.set("DataIndexGroup", std::make_shared<DataIndexGroup>());
		frameResource.set("CurrentLocation", std::make_shared<Location>());
	}

	mPipelineInfo = std::make_shared<CodeRed::PipelineInfo>(mDevice);

	const auto pipelineFactory = mPipelineInfo->pipelineFactory();

	mPipelineInfo->setInputAssemblyState(
		pipelineFactory->createInputAssemblyState(
			{
				CodeRed::InputLayoutElement("POSITION", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 0),
				CodeRed::InputLayoutElement("TEXCOORD", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 1),
				CodeRed::InputLayoutElement("TANGENT", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 2),
				CodeRed::InputLayoutElement("NORMAL", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 3)
			},
			CodeRed::PrimitiveTopology::TriangleList
		)
	);

	mPipelineInfo->setResourceLayout(mResourceLayout);

	if (mDevice->apiVersion() == CodeRed::APIVersion::DirectX12) {
		const auto vShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/");

		mPipelineInfo->setVertexShaderState(
			pipelineFactory->createShaderState(
				CodeRed::ShaderType::Vertex,
				CodeRed::ShaderCompiler::compileToCso(CodeRed::ShaderType::Vertex, vShaderCode)
			)
		);

		mPipelineInfo->setPixelShaderState(
			pipelineFactory->createShaderState(
				CodeRed::ShaderType::Pixel,
				CodeRed::ShaderCompiler::compileToCso(CodeRed::ShaderType::Pixel, fShaderCode)
			)
		);
	}
	else {
		const auto vShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/Vulkan/");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/Vulkan/");

		mPipelineInfo->setVertexShaderState(
			pipelineFactory->createShaderState(
				CodeRed::ShaderType::Vertex,
				CodeRed::ShaderCompiler::compileToSpv(CodeRed::ShaderType::Vertex, vShaderCode)
			)
		);

		mPipelineInfo->setPixelShaderState(
			pipelineFactory->createShaderState(
				CodeRed::ShaderType::Pixel,
				CodeRed::ShaderCompiler::compileToSpv(CodeRed::ShaderType::Pixel, fShaderCode)
			)
		);
	}
}

void LRTR::PhysicalBasedRenderSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	mDrawCalls.clear();

	std::vector<Matrix4x4f> transforms;
	std::vector<SharedMaterial> materials;
	std::vector<std::vector<Vector3f>> vertices(4);
	std::vector<unsigned> indices;

	auto dataIndexGroup = mFrameResources[mCurrentFrameIndex].get<DataIndexGroup>("DataIndexGroup");
	auto currentLocation = mFrameResources[mCurrentFrameIndex].get<Location>("CurrentLocation");

	static const auto ProcessTrianglesMeshComponent = [&](
		const std::shared_ptr<PhysicalBasedMaterial>& physicalBasedMaterial,
		const std::shared_ptr<TrianglesMesh>& trianglesMesh,
		const Matrix4x4f& transform)
	{
		if (!physicalBasedMaterial->visibility()) return;

		//if we mapped the triangles mesh to vertex buffers, we do not need update it again
		//but current version we manager the vertex buffer is fool.
		//because we do not free the old triangle mesh. we need update it.
		if (dataIndexGroup->find(trianglesMesh->identity()) == dataIndexGroup->end()) {
			dataIndexGroup->insert({ trianglesMesh->identity(),
				Location(
					currentLocation->first + vertices[0].size(),
					currentLocation->second + indices.size()) });

			//the zero array is used to fill properties that mesh does not have.
			//when we create the mesh, we will test the size of properties,
			//so the size of properties will be greater or equal than positions
			const auto zeroArray = std::vector<Vector3f>(
				trianglesMesh->positions().size() > trianglesMesh->texCoords().size() ||
				trianglesMesh->positions().size() > trianglesMesh->tangents().size() ||
				trianglesMesh->positions().size() > trianglesMesh->normals().size() ?
				trianglesMesh->positions().size() : 0, Vector3f());

			//position
			vertices[0].insert(vertices[0].end(),
				trianglesMesh->positions().begin(),
				trianglesMesh->positions().end());

			LRTR_INSERT_VERTEX_PROPERTY(
				trianglesMesh->positions().size() > trianglesMesh->texCoords().size(),
				vertices[1], zeroArray, trianglesMesh->texCoords());

			LRTR_INSERT_VERTEX_PROPERTY(
				trianglesMesh->positions().size() > trianglesMesh->tangents().size(),
				vertices[2], zeroArray, trianglesMesh->tangents());

			LRTR_INSERT_VERTEX_PROPERTY(
				trianglesMesh->positions().size() > trianglesMesh->normals().size(),
				vertices[3], zeroArray, trianglesMesh->normals());

			indices.insert(indices.end(),
				trianglesMesh->indices().begin(),
				trianglesMesh->indices().end());
		}

		const auto location = dataIndexGroup->find(trianglesMesh->identity())->second;

		const PhysicalBasedDrawCall drawCall = {
			location.first,
			location.second,
			trianglesMesh->indices().size(),
			false, false, false,
			false, false
		};

		SharedMaterial material;

		//the constant texture of material must be Vector4f
		//for current version, the texture must be constant texture
		//we do not support image texture
		material.BaseColor = std::dynamic_pointer_cast<ConstantTexture<Vector4f>>(physicalBasedMaterial->baseColor())->value();
		material.Roughness = std::dynamic_pointer_cast<ConstantTexture<Vector4f>>(physicalBasedMaterial->roughness())->value();
		material.Metallic = std::dynamic_pointer_cast<ConstantTexture<Vector4f>>(physicalBasedMaterial->metallic())->value();

		mDrawCalls.push_back(drawCall);

		transforms.push_back(transform);
		materials.push_back(material);
	};

	for (const auto& shape : shapes) {
		const auto transform =
			shape.second->hasComponent<TransformWrap>() ? shape.second->component<TransformWrap>()->transform().matrix() :
			Matrix4x4f(1);

		if (shape.second->hasComponent<PhysicalBasedMaterial>() &&
			shape.second->hasComponent<TrianglesMesh>())
		{
			ProcessTrianglesMeshComponent(
				shape.second->component<PhysicalBasedMaterial>(),
				shape.second->component<TrianglesMesh>(),
				transform
			);
		}
	}
}

void LRTR::PhysicalBasedRenderSystem::render(
	const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
{
}

auto LRTR::PhysicalBasedRenderSystem::typeName() const noexcept -> std::string
{
	return "PhysicalBasedRenderSystem";
}

auto LRTR::PhysicalBasedRenderSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(PhysicalBasedRenderSystem);
}
