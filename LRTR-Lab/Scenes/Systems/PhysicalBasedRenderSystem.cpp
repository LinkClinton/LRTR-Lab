#include "PhysicalBasedRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../Scenes/Components/LightSources/PointLightSource.hpp"
#include "../../Scenes/Components/Materials/PhysicalBasedMaterial.hpp"

#include "../../Shared/Textures/ConstantTexture.hpp"
#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"

#define LRTR_IS_CONSTANT_TEXTURE(texture) \
	(std::dynamic_pointer_cast<ConstantTexture<Vector4f>>(texture) != nullptr)

#define LRTR_RESET_BUFFER(buffer, name, binding) \
	if (buffer != mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)) { \
		mFrameResources[mCurrentFrameIndex].set(name, buffer); \
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap")->bindBuffer(buffer, binding); \
	}

namespace LRTR {
	
	struct SharedMaterial {
		Vector4f BaseColor;
		Vector4f Roughness;
		Vector4f Metallic;
	};

	struct SharedLight {
		Vector3f Position;
		Vector3f Intensity;
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
	//resource 1 : light properties
	//resource 2 : transform matrix
	//resource 3 : camera matrix
	//resource 9 : hasBaseColor, HasRoughness, HasOcclusion, HasNormalMap, HasMetallic,
	//eyePosition.x, eyePosition.y, eyePosition.z, nLights, index
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 1),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 2),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 3)
		}, {}, CodeRed::Constant32Bits(10, 9));

	for (auto& frameResource : mFrameResources) {
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

		auto lightBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(SharedLight),
				20,
				CodeRed::MemoryHeap::Upload
			)
		);

		descriptorHeap->bindBuffer(materialBuffer, 0);
		descriptorHeap->bindBuffer(lightBuffer, 1);
		descriptorHeap->bindBuffer(transformBuffer, 2);
		descriptorHeap->bindBuffer(mViewBuffer, 3);

		frameResource.set("DescriptorHeap", descriptorHeap);
		frameResource.set("TransformBuffer", transformBuffer);
		frameResource.set("MaterialBuffer", materialBuffer);
		frameResource.set("LightBuffer", lightBuffer);
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

	mPipelineInfo->setRasterizationState(
		pipelineFactory->createRasterizationState(
			CodeRed::FrontFace::Clockwise,
			CodeRed::CullMode::None,
			CodeRed::FillMode::Solid
		)
	);

	if (mDevice->apiVersion() == CodeRed::APIVersion::DirectX12) {
		const auto vShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/PhysicalBasedRenderSystemVert.hlsl");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/PhysicalBasedRenderSystemFrag.hlsl");

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
	std::vector<SharedLight> lights;
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
		material.BaseColor = Vector4f(std::dynamic_pointer_cast<ConstantTexture<Vector4f>>(physicalBasedMaterial->baseColor())->value());
		material.Roughness = Vector4f(std::dynamic_pointer_cast<ConstantTexture<Vector1f>>(physicalBasedMaterial->roughness())->value());
		material.Metallic = Vector4f(std::dynamic_pointer_cast<ConstantTexture<Vector1f>>(physicalBasedMaterial->metallic())->value());

		mDrawCalls.push_back(drawCall);

		transforms.push_back(transform);
		materials.push_back(material);
	};

	for (const auto& shape : shapes) {
		const auto transform =
			shape.second->hasComponent<TransformWrap>() ? shape.second->component<TransformWrap>() : nullptr;
		
		if (shape.second->hasComponent<PhysicalBasedMaterial>() &&
			shape.second->hasComponent<TrianglesMesh>())
		{
			ProcessTrianglesMeshComponent(
				shape.second->component<PhysicalBasedMaterial>(),
				shape.second->component<TrianglesMesh>(),
				transform != nullptr ? transform->transform().matrix() : Matrix4x4f(1)
			);
		}

		if (shape.second->hasComponent<PointLightSource>()) {
			const auto pointLight = shape.second->component<PointLightSource>();

			lights.push_back({
				transform != nullptr ? transform->translation() : Vector3f(0),
				pointLight->intensity()
				});
		}
	}

	auto transformBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("TransformBuffer");
	auto materialBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("MaterialBuffer");
	auto lightBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("LightBuffer");
	
	transformBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, transformBuffer, transforms.size());
	materialBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, materialBuffer, materials.size());
	lightBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, lightBuffer, lights.size());
	
	LRTR_RESET_BUFFER(transformBuffer, "TransformBuffer", 2);
	LRTR_RESET_BUFFER(materialBuffer, "MaterialBuffer", 0);
	LRTR_RESET_BUFFER(lightBuffer, "LightBuffer", 1);

	CodeRed::ResourceHelper::updateBuffer(transformBuffer, transforms.data(),
		sizeof(Matrix4x4f) * transforms.size());
	CodeRed::ResourceHelper::updateBuffer(materialBuffer, materials.data(),
		sizeof(SharedMaterial) * materials.size());
	CodeRed::ResourceHelper::updateBuffer(lightBuffer, lights.data(),
		sizeof(SharedLight) * lights.size());

	mLights = lights.size();
}

void LRTR::PhysicalBasedRenderSystem::render(
	const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
{
	updatePipeline(frameBuffer);
	updateCamera(camera);

	const auto cameraPosition = getCameraPosition(camera);
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
			cameraPosition.x,
			cameraPosition.y,
			cameraPosition.z,
			static_cast<unsigned>(mLights),
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

auto LRTR::PhysicalBasedRenderSystem::getCameraPosition(const std::shared_ptr<SceneCamera>& camera) const -> Vector3f
{
	if (camera == nullptr || !camera->hasComponent<TransformWrap>()) return Vector3f(0);

	return camera->component<TransformWrap>()->translation();
}
