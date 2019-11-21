#include "CoordinateRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../Components/CoordinateSystem.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"
#include "../../Shared/Color.hpp"

namespace LRTR {
	
	struct AxisVertex {
		Vector3f Position;
		ColorF Color;
	};
	
}

LRTR::CoordinateRenderSystem::CoordinateRenderSystem(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	size_t maxFrameCount) : RenderSystem(sharing, device, maxFrameCount)
{
	mAxisViewBuffer = mDevice->createBuffer(
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
				sizeof(AxisVertex), 
				60,
				CodeRed::MemoryHeap::Upload)
		);

		auto indexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::IndexBuffer(
				sizeof(unsigned),
				60,
				CodeRed::MemoryHeap::Upload)
		);

		auto axisBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(Matrix4x4f),
				10,
				CodeRed::MemoryHeap::Upload)
		);

		descriptorHeap->bindBuffer(axisBuffer, 0);
		descriptorHeap->bindBuffer(mAxisViewBuffer, 1);
		
		frameResource.set("DescriptorHeap", descriptorHeap);
		frameResource.set("VertexBuffer", vertexBuffer);
		frameResource.set("IndexBuffer", indexBuffer);
		frameResource.set("AxisBuffer", axisBuffer);
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

	if (mDevice->apiVersion() == CodeRed::APIVersion::DirectX12) {
		const auto vShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/CoordinateRenderSystemVert.hlsl");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/CoordinateRenderSystemFrag.hlsl");

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
			"./Resources/Shaders/Systems/Vulkan/CoordinateRenderSystemVert.vert");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/Vulkan/CoordinateRenderSystemFrag.frag");

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

void LRTR::CoordinateRenderSystem::update(const StringGroup<std::shared_ptr<Shape>>& shapes, float delta)
{
	std::vector<Matrix4x4f> transforms;
	std::vector<AxisVertex> vertices;
	std::vector<unsigned> indices;

	for (const auto& shape : shapes) {
		if (shape.second->hasComponent<CoordinateSystem>()) {
			const auto axisComponent = shape.second->component<CoordinateSystem>();

			if (!axisComponent->visibility()) continue;
			
			//if we have transform property, we need build the transform matrix
			if (shape.second->hasComponent<TransformWrap>())
				transforms.push_back(shape.second->component<TransformWrap>()->transform().matrix());
			else transforms.push_back(Matrix4x4f(1));

			for (size_t index = 0; index < 3; index++) {
				const auto axis = static_cast<Axis>(index);
				const auto color = axisComponent->color(axis);
				const auto length = axisComponent->length();
				
				vertices.push_back({ Vector3f(0), color });
				vertices.push_back({ axisComponent->axis(axis) * length, color });
				
				indices.push_back(static_cast<unsigned>(vertices.size() - 2));
				indices.push_back(static_cast<unsigned>(vertices.size() - 1));
			}
		}
	}

	auto vertexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("VertexBuffer");
	auto indexBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("IndexBuffer");
	auto axisBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("AxisBuffer");

	//if the buffer can not store all data, we need expand them
	//in current version, we only expand the size to transform size
	if (axisBuffer->count() < transforms.size()) {
		vertexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::VertexBuffer(
				sizeof(AxisVertex),
				transforms.size() * 6,
				CodeRed::MemoryHeap::Upload)
		);

		indexBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::IndexBuffer(
				sizeof(unsigned),
				transforms.size() * 6,
				CodeRed::MemoryHeap::Upload)
		);

		axisBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(Matrix4x4f),
				transforms.size(),
				CodeRed::MemoryHeap::Upload)
		);

		//update the buffer to frame resources
		mFrameResources[mCurrentFrameIndex].set("VertexBuffer", vertexBuffer);
		mFrameResources[mCurrentFrameIndex].set("IndexBuffer", indexBuffer);
		mFrameResources[mCurrentFrameIndex].set("AxisBuffer", axisBuffer);

		//do not forget to update the buffer we bind to descriptor heap.
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap")
			->bindBuffer(axisBuffer, 0);
	}

	mIndexCount = indices.size();

	if (mIndexCount == 0) return;
	CodeRed::ResourceHelper::updateBuffer(vertexBuffer, vertices.data(), sizeof(AxisVertex) * vertices.size());
	CodeRed::ResourceHelper::updateBuffer(indexBuffer, indices.data(), sizeof(unsigned) * indices.size());
	CodeRed::ResourceHelper::updateBuffer(axisBuffer, transforms.data(), sizeof(Matrix4x4f) * transforms.size());
}

void LRTR::CoordinateRenderSystem::render(
	const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	const StringGroup<std::shared_ptr<Shape>>& shapes, 
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

	commandList->drawIndexed(mIndexCount, 1);
	
	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameResources.size();
}

void LRTR::CoordinateRenderSystem::updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const
{
	if (CodeRed::PipelineInfo::isCompatible(mPipelineInfo->renderPass(), frameBuffer) &&
		mPipelineInfo->graphicsPipeline() != nullptr) return;

	mPipelineInfo->setRenderPass(frameBuffer);
	mPipelineInfo->updateState();
}

void LRTR::CoordinateRenderSystem::updateCamera(const std::shared_ptr<SceneCamera>& camera) const
{
	if (camera == nullptr) return;
	
	const auto cameraComponent = camera->component<Projective>();
	const auto viewMatrix = cameraComponent->toScreen().matrix() * 
		camera->component<TransformWrap>()->transform().inverseMatrix();

	CodeRed::ResourceHelper::updateBuffer(mAxisViewBuffer, &viewMatrix);
}
