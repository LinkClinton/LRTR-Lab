#include "WireframeRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Scenes/Components/TrianglesMesh/TrianglesMesh.hpp"
#include "../../Scenes/Components/Materials/WireframeMaterial.hpp"
#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"

#define LRTR_RESET_AND_COPY_BUFFER(buffer, name) \
	if (buffer != mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)) { \
		CodeRed::ResourceHelper::copyBuffer(buffer, mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)); \
		mFrameResources[mCurrentFrameIndex].set(name, vertexBuffer); \
	}

#define LRTR_RESET_BUFFER(buffer, name) \
	if (buffer != mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>(name)) { \
		mFrameResources[mCurrentFrameIndex].set(name, vertexBuffer); \
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
		}, {}, CodeRed::Constant32Bits(4, 2));

	for (auto& frameResource : mFrameResources) {
		auto descriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);

		auto vertexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::VertexBuffer(
				sizeof(Vector3f),
				1000,
				CodeRed::MemoryHeap::Upload
			)
		);

		auto indexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::IndexBuffer(
				sizeof(unsigned),
				3000,
				CodeRed::MemoryHeap::Upload
			)
		);

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
		frameResource.set("VertexBuffer", vertexBuffer);
		frameResource.set("IndexBuffer", indexBuffer);
		frameResource.set("MeshBuffer", meshBuffer);
		frameResource.set("DataIndexGroup", std::make_shared<DataIndexGroup>());
		frameResource.set("CurrentLocation", std::make_shared<Location>());
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

void LRTR::WireframeRenderSystem::update(const StringGroup<std::shared_ptr<Shape>>& shapes, float delta)
{
	mDrawCalls.clear();

	std::vector<Matrix4x4f> transforms;
	std::vector<Vector3f> vertices;
	std::vector<unsigned> indices;

	auto dataIndexGroup = mFrameResources[mCurrentFrameIndex].get<DataIndexGroup>("DataIndexGroup");
	auto currentLocation = mFrameResources[mCurrentFrameIndex].get<Location>("CurrentLocation");

	static const auto ProcessTrianglesMeshComponent = [&](
		const std::shared_ptr<WireframeMaterial>& wireframeMaterial,
		const std::shared_ptr<TrianglesMesh>& trianglesMesh,
		const Matrix4x4f& transform)
	{
		if (!wireframeMaterial->visibility()) return;

		//if we mapped the triangles mesh to vertex buffers, we do not need update it again
		//but current version we manager the vertex buffer is fool.
		//because we do not free the old triangle mesh. we need update it.
		if (dataIndexGroup->find(trianglesMesh->identity()) == dataIndexGroup->end()) {
			dataIndexGroup->insert({ trianglesMesh->identity(),
				Location(
					currentLocation->first + vertices.size(),
					currentLocation->second + indices.size()) });

			vertices.insert(vertices.end(),
				trianglesMesh->vertices().begin(),
				trianglesMesh->vertices().end());

			indices.insert(indices.end(),
				trianglesMesh->indices().begin(),
				trianglesMesh->indices().end());
		}

		const auto location = dataIndexGroup->find(trianglesMesh->identity())->second;

		mDrawCalls.push_back({
			location.first,
			location.second,
			trianglesMesh->indices().size(),
			wireframeMaterial->color() });

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

	auto vertexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("VertexBuffer");
	auto indexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("IndexBuffer");
	auto meshBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("MeshBuffer");

	vertexBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, vertexBuffer, currentLocation->first + vertices.size());
	indexBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, indexBuffer, currentLocation->second + indices.size());
	meshBuffer = CodeRed::ResourceHelper::expandBuffer(mDevice, meshBuffer, transforms.size());

	LRTR_RESET_AND_COPY_BUFFER(vertexBuffer, "VertexBuffer");
	LRTR_RESET_AND_COPY_BUFFER(indexBuffer, "IndexBuffer");
	LRTR_RESET_BUFFER(meshBuffer, "MeshBuffer");

	CodeRed::ResourceHelper::updateBuffer(vertexBuffer, vertices.data(), sizeof(Vector3f) * currentLocation->first,
		sizeof(Vector3f) * vertices.size());
	CodeRed::ResourceHelper::updateBuffer(indexBuffer, indices.data(), sizeof(unsigned) * currentLocation->second,
		sizeof(unsigned) * indices.size());
	CodeRed::ResourceHelper::updateBuffer(meshBuffer, transforms.data(),
		sizeof(Matrix4x4f) * transforms.size());

	currentLocation->first = currentLocation->first + vertices.size();
	currentLocation->second = currentLocation->second + indices.size();
}

void LRTR::WireframeRenderSystem::render(
	const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
{
	updatePipeline(frameBuffer);
	updateCamera(camera);

	const auto descriptorHeap = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap");
	const auto vertexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("VertexBuffer");
	const auto indexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("IndexBuffer");

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(descriptorHeap);

	commandList->setVertexBuffer(vertexBuffer);
	commandList->setIndexBuffer(indexBuffer);

	for (size_t index = 0; index < mDrawCalls.size();index++) {
		const auto& drawCall = mDrawCalls[index];
		
		commandList->setConstant32Bits({
			drawCall.Color.Red,
			drawCall.Color.Green,
			drawCall.Color.Blue,
			drawCall.Color.Alpha
		});

		commandList->drawIndexed(drawCall.IndexCount, 1,
			drawCall.StartIndexLocation,
			drawCall.StartVertexLocation,
			index);
	}
	
	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameResources.size();
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
