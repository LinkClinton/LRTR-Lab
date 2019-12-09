#include "PhysicalBasedRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../Scenes/Components/Materials/PhysicalBasedMaterial.hpp"

#include "../../Shared/Textures/ConstantTexture.hpp"
#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"

#define LRTR_IS_CONSTANT_TEXTURE(texture) \
	(std::dynamic_pointer_cast<ConstantTexture<Vector4f>>(texture) != nullptr)

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
	//resource 8 : hasBaseColor, HasRoughness, HasOcclusion, HasNormalMap, HasMetallic, index
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer,1),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer,2)
		}, {}, CodeRed::Constant32Bits(6, 8));

	for (auto& frameResource : mFrameResources) {
		auto vertexBuffers = std::make_shared<std::vector<std::shared_ptr<CodeRed::GpuBuffer>>>(4);
		auto descriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);

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
		frameResource.set("TransformBuffer", transformBuffer);
		frameResource.set("MaterialBuffer", materialBuffer);
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
	
	static const auto ProcessTrianglesMeshComponent = [&](
		const std::shared_ptr<PhysicalBasedMaterial>& physicalBasedMaterial,
		const std::shared_ptr<TrianglesMesh>& trianglesMesh,
		const Matrix4x4f& transform)
	{
		if (!physicalBasedMaterial->visibility()) return;

		const PhysicalBasedDrawCall drawCall = {
			trianglesMesh
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

	auto transformBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("TransformBuffer");
	auto materialBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("MaterialBuffer");
	
	transformBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, transformBuffer, transforms.size());
	materialBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, materialBuffer, materials.size());
	
	LRTR_RESET_BUFFER(transformBuffer, "TransformBuffer");
	LRTR_RESET_BUFFER(materialBuffer, "MaterialBuffer");

	CodeRed::ResourceHelper::updateBuffer(transformBuffer, transforms.data(),
		sizeof(Matrix4x4f) * transforms.size());
	CodeRed::ResourceHelper::updateBuffer(materialBuffer, materials.data(),
		sizeof(SharedMaterial) * materials.size());
}

void LRTR::PhysicalBasedRenderSystem::render(
	const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
{
	updatePipeline(frameBuffer);
	updateCamera(camera);

	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		mRuntimeSharing->assetManager()->components().at("MeshData"));

	//update the vertex buffer we use,
	//in render function, we do not render anything
	//so we can update vertex buffer directly
	meshDataAssetComponent->beginAllocating();

	for (const auto& drawCall : mDrawCalls)
		meshDataAssetComponent->allocate(drawCall.Mesh);

	meshDataAssetComponent->endAllocating();

	const auto descriptorHeap = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap");
	
	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(descriptorHeap);

	commandList->setVertexBuffers({
		meshDataAssetComponent->positions(),
		meshDataAssetComponent->texCoords(),
		meshDataAssetComponent->tangents(),
		meshDataAssetComponent->normals()
		});
	
	commandList->setIndexBuffer(meshDataAssetComponent->indices());

	for (size_t index = 0; index < mDrawCalls.size(); index++) {
		const auto drawCall = mDrawCalls[index];
		const auto meshDataInfo = meshDataAssetComponent->get(drawCall.Mesh);

		commandList->setConstant32Bits({
			drawCall.HasBaseColor,
			drawCall.HasRoughness,
			drawCall.HasOcclusion,
			drawCall.HasNormalMap,
			drawCall.HasMetallic,
			static_cast<unsigned>(index)
		});

		commandList->drawIndexed(meshDataInfo.IndexCount, 1,
			meshDataInfo.StartIndexLocation,
			meshDataInfo.StartVertexLocation,
			0);
	}

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameResources.size();
}

auto LRTR::PhysicalBasedRenderSystem::typeName() const noexcept -> std::string
{
	return "PhysicalBasedRenderSystem";
}

auto LRTR::PhysicalBasedRenderSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(PhysicalBasedRenderSystem);
}

void LRTR::PhysicalBasedRenderSystem::updatePipeline(
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const
{
	if (CodeRed::PipelineInfo::isCompatible(mPipelineInfo->renderPass(), frameBuffer) &&
		mPipelineInfo->graphicsPipeline() != nullptr) return;

	mPipelineInfo->setRenderPass(frameBuffer);
	mPipelineInfo->updateState();
}

void LRTR::PhysicalBasedRenderSystem::updateCamera(
	const std::shared_ptr<SceneCamera>& camera) const
{
	if (camera == nullptr) return;

	const auto cameraComponent = camera->component<Projective>();
	const auto viewMatrix = cameraComponent->toScreen().matrix() *
		camera->component<TransformWrap>()->transform().inverseMatrix();

	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, &viewMatrix, sizeof(Matrix4x4f));
}
