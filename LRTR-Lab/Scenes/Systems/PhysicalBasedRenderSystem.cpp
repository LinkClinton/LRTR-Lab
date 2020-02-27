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
#include "../../Shared/Textures/ImageTexture.hpp"

#include "../../Workflow/Shaders/CompileShaderWorkflow.hpp"

#define LRTR_RESET_BUFFER(buffer, name, binding) \
	if (buffer != mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)) { \
		mFrameResources[mCurrentFrameIndex].set(name, buffer); \
	}

namespace LRTR {
	
	struct SharedMaterial {
		Vector4f BaseColor;
		Vector4f Roughness;
		Vector4f Metallic;
		Vector4f Emissive;
	};

	struct SharedLight {
		Vector4f Position;
		Vector4f Intensity;

		float FarPlane;
		unsigned Index;
		unsigned Type;
		unsigned Unused;
	};
	
}

LRTR::PointShadowMap::PointShadowMap(
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const size_t extent, const size_t length)
{
	Texture = device->createTexture(
		CodeRed::ResourceInfo::CubeMapArray(extent, extent, length, CodeRed::PixelFormat::Depth32BitFloat, 1,
			CodeRed::ResourceUsage::DepthStencil)
	);

	for (size_t index = 0; index < length; index++) {
		PointShadowFrameBuffer frameBuffer;

		for (size_t face = 0; face < 6; face++) {
			frameBuffer[face] = device->createFrameBuffer(nullptr, Texture->reference(
				CodeRed::TextureRefInfo(
					CodeRed::ValueRange<size_t>(0, 1),
					CodeRed::ValueRange<size_t>(index * 6 + face, index * 6 + face + 1),
					CodeRed::TextureRefUsage::Common,
					CodeRed::PixelFormat::Depth32BitFloat
				)));
		}

		FrameBuffers.push_back(frameBuffer);
	}
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
	
	mSampler = mDevice->createSampler(
		CodeRed::SamplerInfo(16,
			CodeRed::AddressMode::Repeat,
			CodeRed::AddressMode::Repeat,
			CodeRed::AddressMode::Repeat)
	);

	//resource 0 : material properties
	//resource 1 : light properties
	//resource 2 : transform matrix
	//resource 3 : camera matrix
	//resource 4 : metallic texture
	//resource 5 : baseColor texture
	//resource 6 : roughness texture
	//resource 7 : occlusion texture
	//resource 8 : normalMap texture
	//resource 9 : emissive texture
	//resource 10 : irradiance map
	//resource 11 : pre filtering map
	//resource 12 : pre computingBRDF map
	//resource 13 : point shadow map array
	//resource 14 : sampler
	//resource 15 : hasEnvironmentLight, hasBaseColor, HasRoughness, HasOcclusion, HasNormalMap, HasMetallic, HasEmissive,
	//eyePosition.x, eyePosition.y, eyePosition.z, MipLevels, nLights, index
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 1),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 2),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 3),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 4),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 5),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 6),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 7),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 8),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 9),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 10),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 11),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 12),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 13)
		}, {
			CodeRed::SamplerLayoutElement(mSampler, 0, 1)
		}, CodeRed::Constant32Bits(13, 0, 2));

	for (auto& frameResource : mFrameResources) {
		auto descriptorHeapPool = std::make_shared<std::vector<std::shared_ptr<CodeRed::GpuDescriptorHeap>>>();
		
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

		frameResource.set("DescriptorHeapPool", descriptorHeapPool);
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
			CodeRed::FrontFace::CounterClockwise,
			CodeRed::CullMode::Back,
			CodeRed::FillMode::Solid
		)
	);
	
	CompileShaderWorkflow workflow;

#ifdef SHADER_SOURCE_HLSL
	const auto sourceLanguage = SourceLanguage::eHLSL;
#else
	const auto sourceLanguage = SourceLanguage::eGLSL;
#endif
	const auto targetLanguage = mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		TargetLanguage::eDXIL : TargetLanguage::eSPIRV;

	const auto vShaderFile =
		sourceLanguage == SourceLanguage::eHLSL ?
		"./Resources/Shaders/Systems/DirectX12/PhysicalBasedRenderSystemVert.hlsl" :
		"./Resources/Shaders/Systems/Vulkan/PhysicalBasedRenderSystemVert.vert";

	const auto fShaderFile =
		sourceLanguage == SourceLanguage::eHLSL ?
		"./Resources/Shaders/Systems/DirectX12/PhysicalBasedRenderSystemFrag.hlsl" :
		"./Resources/Shaders/Systems/Vulkan/PhysicalBasedRenderSystemFrag.frag";

	mPipelineInfo->setVertexShaderState(
		pipelineFactory->createShaderState(
			CodeRed::ShaderType::Vertex,
			workflow.start({ CompileShaderInput(
				vShaderFile,
				CodeRed::ShaderType::Vertex,
				sourceLanguage,
				targetLanguage
			) })
		)
	);

	mPipelineInfo->setPixelShaderState(
		pipelineFactory->createShaderState(
			CodeRed::ShaderType::Pixel,
			workflow.start({ CompileShaderInput(
				fShaderFile,
				CodeRed::ShaderType::Pixel,
				sourceLanguage,
				targetLanguage
			) })
		)
	);

	// in this version, we only support 2 point light for test
	mPointShadowMap = std::make_shared<PointShadowMap>(mDevice, 1024, 5);
	
	mPointShadowMapWorkflow = std::make_shared<PointShadowMapWorkflow>(mDevice);
}

void LRTR::PhysicalBasedRenderSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	mPointShadowAreas.clear();
	mShadowCastInfos.clear();
	mDrawCalls.clear();
	
	std::vector<Matrix4x4f> transforms;
	std::vector<SharedLight> lights;
	std::vector<SharedMaterial> materials;

	auto descriptorHeapPool = mFrameResources[mCurrentFrameIndex].
		get<std::vector<std::shared_ptr<CodeRed::GpuDescriptorHeap>>>("DescriptorHeapPool");
	
	static const auto ProcessTrianglesMeshComponent = [&](
		const std::shared_ptr<PhysicalBasedMaterial>& physicalBasedMaterial,
		const std::shared_ptr<TrianglesMesh>& trianglesMesh,
		const Matrix4x4f& transform,
		const size_t index)
	{
		if (!physicalBasedMaterial->visibility()) return;

		const auto descriptorHeap = (*descriptorHeapPool)[index];
		
		PhysicalBasedDrawCall drawCall = {
			trianglesMesh
		};

		SharedMaterial material;

		material.BaseColor = physicalBasedMaterial->baseColorFactor()->value();
		material.Roughness = physicalBasedMaterial->roughnessFactor()->value();
		material.Metallic = physicalBasedMaterial->metallicFactor()->value();
		material.Emissive = physicalBasedMaterial->emissiveFactor()->value();
		
		drawCall.HasMetallic = physicalBasedMaterial->metallicTexture() != nullptr;
		drawCall.HasBaseColor = physicalBasedMaterial->baseColorTexture() != nullptr;
		drawCall.HasRoughness = physicalBasedMaterial->roughnessTexture() != nullptr;
		drawCall.HasOcclusion = physicalBasedMaterial->occlusionTexture() != nullptr;
		drawCall.HasNormalMap = physicalBasedMaterial->normalMapTexture() != nullptr;
		drawCall.HasEmissive = physicalBasedMaterial->emissiveTexture() != nullptr;

		if (drawCall.HasMetallic) descriptorHeap->bindTexture(physicalBasedMaterial->metallicTexture()->value(), 4);
		if (drawCall.HasBaseColor) descriptorHeap->bindTexture(physicalBasedMaterial->baseColorTexture()->value(), 5);
		if (drawCall.HasRoughness) descriptorHeap->bindTexture(physicalBasedMaterial->roughnessTexture()->value(), 6);
		if (drawCall.HasOcclusion) descriptorHeap->bindTexture(physicalBasedMaterial->occlusionTexture()->value(), 7);
		if (drawCall.HasNormalMap) descriptorHeap->bindTexture(physicalBasedMaterial->normalMapTexture()->value(), 8);
		if (drawCall.HasEmissive) descriptorHeap->bindTexture(physicalBasedMaterial->emissiveTexture()->value(), 9);

		// only for test, we need find a good way to support 
		if (mEnvironmentLight.Irradiance != nullptr) descriptorHeap->bindTexture(
			mEnvironmentLight.Irradiance->reference(CodeRed::TextureRefUsage::CubeMap), 10);

		if (mEnvironmentLight.PreFiltering != nullptr) descriptorHeap->bindTexture(
			mEnvironmentLight.PreFiltering->reference(CodeRed::TextureRefUsage::CubeMap), 11);

		if (mEnvironmentLight.PreComputingBRDF != nullptr) descriptorHeap->bindTexture(
			mEnvironmentLight.PreComputingBRDF, 12);

		descriptorHeap->bindTexture(mPointShadowMap->Texture->reference(
			CodeRed::TextureRefInfo(
				CodeRed::TextureRefUsage::CubeMap, 
				CodeRed::PixelFormat::Red32BitFloat)), 13);

		// only cast shadow that enable ShadowCast
		if (physicalBasedMaterial->isCast()) mShadowCastInfos.push_back({ trianglesMesh, index });

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
			if (descriptorHeapPool->size() == mDrawCalls.size()) 
				descriptorHeapPool->push_back(mDevice->createDescriptorHeap(mResourceLayout));
			
			ProcessTrianglesMeshComponent(
				shape.second->component<PhysicalBasedMaterial>(),
				shape.second->component<TrianglesMesh>(),
				transform != nullptr ? transform->transform().matrix() : Matrix4x4f(1),
				mDrawCalls.size()
			);
		}

		if (shape.second->hasComponent<PointLightSource>()) {
			const auto pointLight = shape.second->component<PointLightSource>();
			
			//if index is zero means we do not cast shadow
			//if is not zero, it indicate the index of shadow map.
			lights.push_back({
				transform != nullptr ? Vector4f(transform->translation(), 1.0f) : Vector4f(0),
				Vector4f(pointLight->intensity(), 1.0f),
				25.0f,
				pointLight->isCast() ? static_cast<unsigned>(mPointShadowAreas.size() + 1) : 0,
				0, });

			if (pointLight->isCast()) 
				mPointShadowAreas.push_back({ mPointShadowMap->FrameBuffers[mPointShadowAreas.size()], lights.back().Position, 25.0f });
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

	for (size_t index = 0; index < mDrawCalls.size(); index++) {
		const auto descriptorHeap = (*descriptorHeapPool)[index];

		descriptorHeap->bindBuffer(materialBuffer, 0);
		descriptorHeap->bindBuffer(lightBuffer, 1);
		descriptorHeap->bindBuffer(transformBuffer, 2);
		descriptorHeap->bindBuffer(mViewBuffer, 3);
	}

	CodeRed::ResourceHelper::updateBuffer(transformBuffer, transforms.data(),
		sizeof(Matrix4x4f) * transforms.size());
	CodeRed::ResourceHelper::updateBuffer(materialBuffer, materials.data(),
		sizeof(SharedMaterial) * materials.size());
	CodeRed::ResourceHelper::updateBuffer(lightBuffer, lights.data(),
		sizeof(SharedLight) * lights.size());

	mLights = lights.size();

	//update the vertex buffer we use
	//we update the buffer to avoid issue 1
	//we will update all systems before rendering
	//https://github.com/LinkClinton/LRTR-Lab/issues/1
	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		mRuntimeSharing->assetManager()->components().at("MeshData"));

	meshDataAssetComponent->beginAllocating();

	for (const auto& drawCall : mDrawCalls)
		meshDataAssetComponent->allocate(drawCall.Mesh);

	meshDataAssetComponent->endAllocating();
}

void LRTR::PhysicalBasedRenderSystem::render(
	const std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>& commandLists,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
{
	updatePipeline(frameBuffer);
	updateCamera(camera);
	
	// pre build shadow map for lights
	// in this version, we only test on point shadow map
	mPointShadowMapWorkflow->start({
		PointShadowMapInput(
			commandLists[0], mPointShadowMap->Texture,
			mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("TransformBuffer"),
			mRuntimeSharing, mPointShadowAreas , mShadowCastInfos) });
		
	const auto cameraPosition = getCameraPosition(camera);
	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		mRuntimeSharing->assetManager()->components().at("MeshData"));
	
	const auto descriptorHeapPool = mFrameResources[mCurrentFrameIndex]
		.get<std::vector<std::shared_ptr<CodeRed::GpuDescriptorHeap>>>("DescriptorHeapPool");

	const auto commandList = commandLists[1];
	
	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	
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
		const auto mipLevels = hasEnvironmentLight() ? mEnvironmentLight.PreFiltering->mipLevels() : 0;
		
		commandList->setDescriptorHeap((*descriptorHeapPool)[index]);

		commandList->setConstant32Bits({
			hasEnvironmentLight(),
			drawCall.HasBaseColor,
			drawCall.HasRoughness,
			drawCall.HasOcclusion,
			drawCall.HasNormalMap,
			drawCall.HasMetallic,
			drawCall.HasEmissive,
			cameraPosition.x,
			cameraPosition.y,
			cameraPosition.z,
			static_cast<unsigned>(mipLevels),
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

void LRTR::PhysicalBasedRenderSystem::setEnvironmentLight(const EnvironmentLight& light)
{
	mEnvironmentLight = light;
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

auto LRTR::PhysicalBasedRenderSystem::hasEnvironmentLight() const noexcept -> bool
{
	return mEnvironmentLight.Irradiance != nullptr && 
		mEnvironmentLight.PreFiltering != nullptr &&
		mEnvironmentLight.PreComputingBRDF != nullptr;
}
