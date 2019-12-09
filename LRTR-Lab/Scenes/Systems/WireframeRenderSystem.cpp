#include "WireframeRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"
#include "../../Scenes/Components/Materials/WireframeMaterial.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"

#define LRTR_RESET_BUFFER(buffer, name) \
	if (buffer != mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)) { \
		mFrameResources[mCurrentFrameIndex].set(name, buffer); \
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap")->bindBuffer(buffer, 0); \
	}

LRTR::WireframeRenderSystem::WireframeRenderSystem(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device, 
	size_t maxFrameCount) : RenderSystem(sharing, device, maxFrameCount)
{
	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(
			sizeof(Matrix4x4f)
		)
	);

	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer,1)
		}, {}, CodeRed::Constant32Bits(5, 2));

	for (auto& frameResource : mFrameResources) {
		auto descriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);

		auto meshBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(Matrix4x4f),
				100,
				CodeRed::MemoryHeap::Upload
			)
		);

		descriptorHeap->bindBuffer(meshBuffer, 0);
		descriptorHeap->bindBuffer(mViewBuffer, 1);

		frameResource.set("DescriptorHeap", descriptorHeap);
		frameResource.set("MeshBuffer", meshBuffer);
	}

	mPipelineInfo = std::make_shared<CodeRed::PipelineInfo>(mDevice);

	const auto pipelineFactory = mPipelineInfo->pipelineFactory();

	mPipelineInfo->setInputAssemblyState(
		pipelineFactory->createInputAssemblyState(
			{
				CodeRed::InputLayoutElement("POSITION", CodeRed::PixelFormat::RedGreenBlue32BitFloat)
			},
			CodeRed::PrimitiveTopology::TriangleList
		)
	);

	mPipelineInfo->setRasterizationState(
		pipelineFactory->createRasterizationState(
			CodeRed::FrontFace::Clockwise,
			CodeRed::CullMode::None,
			CodeRed::FillMode::Wireframe
		)
	);

	mPipelineInfo->setResourceLayout(mResourceLayout);

	if (mDevice->apiVersion() == CodeRed::APIVersion::DirectX12) {
		const auto vShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/WireframeRenderSystemVert.hlsl");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/WireframeRenderSystemFrag.hlsl");

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
			"./Resources/Shaders/Systems/Vulkan/WireframeRenderSystemVert.vert");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/Vulkan/WireframeRenderSystemFrag.frag");

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

void LRTR::WireframeRenderSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	mDrawCalls.clear();

	std::vector<Matrix4x4f> transforms;
	
	static const auto ProcessTrianglesMeshComponent = [&](
		const std::shared_ptr<WireframeMaterial>& wireframeMaterial,
		const std::shared_ptr<TrianglesMesh>& trianglesMesh,
		const Matrix4x4f& transform)
	{
		if (!wireframeMaterial->visibility()) return;

		mDrawCalls.push_back({
			trianglesMesh,
			wireframeMaterial->color()
		});

		transforms.push_back(transform);
	};

	for (const auto& shape : shapes) {
		const auto transform =
			shape.second->hasComponent<TransformWrap>() ? shape.second->component<TransformWrap>()->transform().matrix() :
			Matrix4x4f(1);

		if (shape.second->hasComponent<TrianglesMesh>() &&
			shape.second->hasComponent<WireframeMaterial>())
		{
			ProcessTrianglesMeshComponent(
				shape.second->component<WireframeMaterial>(),
				shape.second->component<TrianglesMesh>(),
				transform
			);
		}
	}

	auto meshBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("MeshBuffer");

	meshBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, meshBuffer, transforms.size());

	LRTR_RESET_BUFFER(meshBuffer, "MeshBuffer");

	CodeRed::ResourceHelper::updateBuffer(meshBuffer, transforms.data(),
		sizeof(Matrix4x4f) * transforms.size());
}

void LRTR::WireframeRenderSystem::render(
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
	const auto vertexBuffer = meshDataAssetComponent->positions();
	const auto indexBuffer = meshDataAssetComponent->indices();

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(descriptorHeap);

	commandList->setVertexBuffer(vertexBuffer);
	commandList->setIndexBuffer(indexBuffer);
	
	for (size_t index = 0; index < mDrawCalls.size();index++) {
		const auto drawCall = mDrawCalls[index];
		const auto meshDataInfo = meshDataAssetComponent->get(drawCall.Mesh);
		
		commandList->setConstant32Bits({
			drawCall.Color.Red,
			drawCall.Color.Green,
			drawCall.Color.Blue,
			drawCall.Color.Alpha,
			static_cast<unsigned>(index)
		});

		commandList->drawIndexed(meshDataInfo.IndexCount, 1,
			meshDataInfo.StartIndexLocation,
			meshDataInfo.StartVertexLocation,
			0);
	}
	
	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameResources.size();
}

auto LRTR::WireframeRenderSystem::typeName() const noexcept -> std::string
{
	return "WireframeRenderSystem";
}

auto LRTR::WireframeRenderSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(WireframeRenderSystem);
}

void LRTR::WireframeRenderSystem::updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const
{
	if (CodeRed::PipelineInfo::isCompatible(mPipelineInfo->renderPass(), frameBuffer) &&
		mPipelineInfo->graphicsPipeline() != nullptr) return;

	mPipelineInfo->setRenderPass(frameBuffer);
	mPipelineInfo->updateState();
}

void LRTR::WireframeRenderSystem::updateCamera(const std::shared_ptr<SceneCamera>& camera) const
{
	if (camera == nullptr) return;

	const auto cameraComponent = camera->component<Projective>();
	const auto viewMatrix = cameraComponent->toScreen().matrix() *
		camera->component<TransformWrap>()->transform().inverseMatrix();

	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, &viewMatrix, sizeof(Matrix4x4f));
}
