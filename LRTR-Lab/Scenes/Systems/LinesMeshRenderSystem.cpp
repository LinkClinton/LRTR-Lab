#include "LinesMeshRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../Components/LinesMesh/CoordinateSystem.hpp"
#include "../Components/LinesMesh/LinesGrid.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"
#include "../../Shared/Color.hpp"

#include "../../Workflow/Shaders/CompileShaderWorkflow.hpp"

namespace LRTR {

	struct LineVertex {
		Vector3f Position;
		ColorF Color;
	};

}

LRTR::LinesMeshRenderSystem::LinesMeshRenderSystem(
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
		});

	for (auto& frameResource : mFrameResources) {
		auto descriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);
		auto vertexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::VertexBuffer(
				sizeof(LineVertex),
				200,
				CodeRed::MemoryHeap::Upload)
		);

		auto indexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::IndexBuffer(
				sizeof(unsigned),
				200,
				CodeRed::MemoryHeap::Upload)
		);

		auto lineBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(Matrix4x4f),
				100,
				CodeRed::MemoryHeap::Upload)
		);

		descriptorHeap->bindBuffer(lineBuffer, 0);
		descriptorHeap->bindBuffer(mViewBuffer, 1);

		frameResource.set("DescriptorHeap", descriptorHeap);
		frameResource.set("VertexBuffer", vertexBuffer);
		frameResource.set("IndexBuffer", indexBuffer);
		frameResource.set("LineBuffer", lineBuffer);
	}

	mPipelineInfo = std::make_shared<CodeRed::PipelineInfo>(mDevice);

	const auto pipelineFactory = mPipelineInfo->pipelineFactory();
	
	mPipelineInfo->setInputAssemblyState(
		pipelineFactory->createInputAssemblyState(
			{
				CodeRed::InputLayoutElement("POSITION", CodeRed::PixelFormat::RedGreenBlue32BitFloat),
				CodeRed::InputLayoutElement("COLOR", CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat)
			},
			CodeRed::PrimitiveTopology::LineList
		)
	);

	mPipelineInfo->setResourceLayout(mResourceLayout);
	
	mPipelineInfo->setBlendState(pipelineFactory->createBlendState(2));
	
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
		"./Resources/Shaders/Systems/DirectX12/LinesMeshRenderSystemVert.hlsl" :
		"./Resources/Shaders/Systems/Vulkan/LinesMeshRenderSystemVert.vert";

	const auto fShaderFile =
		sourceLanguage == SourceLanguage::eHLSL ?
		"./Resources/Shaders/Systems/DirectX12/LinesMeshRenderSystemFrag.hlsl" :
		"./Resources/Shaders/Systems/Vulkan/LinesMeshRenderSystemFrag.frag";

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
}

void LRTR::LinesMeshRenderSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	std::vector<Matrix4x4f> transforms;
	std::vector<LineVertex> vertices;
	std::vector<unsigned> indices;

	static const auto ProcessLinesMeshComponent = [&](
		const Matrix4x4f& transform,
		const std::shared_ptr<LinesMesh>& component)
	{
		if (!component->IsRendered) return;

		for (size_t index = 0; index < component->size(); index++) {
			transforms.push_back(transform);

			const auto line = component->line(index);

			vertices.push_back({ line.Begin, line.Color });
			vertices.push_back({ line.End, line.Color });

			indices.push_back(static_cast<unsigned>(vertices.size() - 2));
			indices.push_back(static_cast<unsigned>(vertices.size() - 1));
		}
	};
	
	for (const auto& shape : shapes) {
		const auto transform =
			shape.second->hasComponent<TransformWrap>() ? shape.second->component<TransformWrap>()->transform().matrix() :
			Matrix4x4f(1);
		
		if (shape.second->hasComponent<CoordinateSystem>())
			ProcessLinesMeshComponent(transform, shape.second->component<CoordinateSystem>());

		if (shape.second->hasComponent<LinesMesh>())
			ProcessLinesMeshComponent(transform, shape.second->component<LinesMesh>());

		if (shape.second->hasComponent<LinesGrid>())
			ProcessLinesMeshComponent(transform, shape.second->component<LinesGrid>());
	}

	auto vertexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("VertexBuffer");
	auto indexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("IndexBuffer");
	auto lineBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("LineBuffer");

	//if the buffer can not store all data, we need expand them
	//in current version, we only expand the size to transform size
	if (lineBuffer->count() < transforms.size()) {
		const size_t expandLength = 100;
		
		auto newBufferCount = lineBuffer->count();

		while (newBufferCount < transforms.size()) 
			newBufferCount = newBufferCount + expandLength;
		
		vertexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::VertexBuffer(
				sizeof(LineVertex),
				newBufferCount * 2,
				CodeRed::MemoryHeap::Upload)
		);

		indexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::IndexBuffer(
				sizeof(unsigned),
				newBufferCount * 2,
				CodeRed::MemoryHeap::Upload)
		);

		lineBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(Matrix4x4f),
				newBufferCount,
				CodeRed::MemoryHeap::Upload)
		);

		//update the buffer to frame resources
		mFrameResources[mCurrentFrameIndex].set("VertexBuffer", vertexBuffer);
		mFrameResources[mCurrentFrameIndex].set("IndexBuffer", indexBuffer);
		mFrameResources[mCurrentFrameIndex].set("LineBuffer", lineBuffer);

		//do not forget to update the buffer we bind to descriptor heap.
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap")
			->bindBuffer(lineBuffer, 0);
	}

	mIndexCount = indices.size();

	if (mIndexCount == 0) return;

	CodeRed::ResourceHelper::updateBuffer(vertexBuffer, vertices.data(), sizeof(LineVertex) * vertices.size());
	CodeRed::ResourceHelper::updateBuffer(indexBuffer, indices.data(), sizeof(unsigned) * indices.size());
	CodeRed::ResourceHelper::updateBuffer(lineBuffer, transforms.data(), sizeof(Matrix4x4f) * transforms.size());
}

void LRTR::LinesMeshRenderSystem::render(
	const std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>& commandLists,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer,
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
{
	updatePipeline(frameBuffer);
	updateCamera(camera);

	const auto descriptorHeap = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap");
	const auto vertexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("VertexBuffer");
	const auto indexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("IndexBuffer");

	const auto commandList = commandLists[1];
	
	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(descriptorHeap);

	commandList->setVertexBuffer(vertexBuffer);
	commandList->setIndexBuffer(indexBuffer);

	commandList->drawIndexed(mIndexCount, 1);

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameResources.size();
}

auto LRTR::LinesMeshRenderSystem::typeName() const noexcept -> std::string
{
	return "LinesMeshRenderSystem";
}

auto LRTR::LinesMeshRenderSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(LinesMeshRenderSystem);
}

void LRTR::LinesMeshRenderSystem::updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const
{
	if (CodeRed::PipelineInfo::isCompatible(mPipelineInfo->renderPass(), frameBuffer) &&
		mPipelineInfo->graphicsPipeline() != nullptr) return;

	mPipelineInfo->setRenderPass(frameBuffer);
	mPipelineInfo->updateState();
}

void LRTR::LinesMeshRenderSystem::updateCamera(const std::shared_ptr<SceneCamera>& camera) const
{
	if (camera == nullptr) return;

	const auto cameraComponent = camera->component<Projective>();
	const auto viewMatrix = cameraComponent->toScreen().matrix() *
		camera->component<TransformWrap>()->transform().inverseMatrix();

	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, &viewMatrix, sizeof(Matrix4x4f));
}
