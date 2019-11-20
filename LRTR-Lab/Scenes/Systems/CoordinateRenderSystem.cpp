#include "CoordinateRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../Components/CoordinateSystem.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"
#include "../../Shared/Color.hpp"

namespace LRTR {
	
	struct AxisBufferData {
		Matrix4x4f Transform;
		ColorF Color;
	};
	
}

LRTR::CoordinateRenderSystem::CoordinateRenderSystem(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	size_t maxFrameCount) : RenderSystem(sharing, device, maxFrameCount)
{
	mAxisVertexBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::VertexBuffer(
			sizeof(Vector3f), 8
		)
	);

	mAxisIndexBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::IndexBuffer(
			sizeof(unsigned), 36
		)
	);

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
		auto axisBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(AxisBufferData), 10));
		
		descriptorHeap->bindBuffer(axisBuffer, 0);
		descriptorHeap->bindBuffer(mAxisViewBuffer, 1);
		
		frameResource.set("DescriptorHeap", descriptorHeap);
		frameResource.set("AxisBuffer", axisBuffer);
	}

	std::vector<Vector3f> vertices = {
		Vector3f(0.f, -0.5f, -0.5f),
		Vector3f(0.f, +0.5f, -0.5f),
		Vector3f(1.f, +0.5f, -0.5f),
		Vector3f(1.f, -0.5f, -0.5f),
		Vector3f(0.f, -0.5f, +0.5f),
		Vector3f(0.f, +0.5f, +0.5f),
		Vector3f(1.f, +0.5f, +0.5f),
		Vector3f(1.f, -0.5f, +0.5f)
	};

	std::vector<unsigned> indices = {
		0, 2, 1, 0, 3, 2,
		4, 5, 6, 4, 6, 7,
		4, 1, 5, 4, 0, 1,
		3, 6, 2, 3, 7, 6,
		1, 6, 5, 1, 2, 6,
		4, 3, 0, 4, 7, 3
	};

	CodeRed::ResourceHelper::updateBuffer(
		mRuntimeSharing->device(),
		mRuntimeSharing->allocator(),
		mAxisVertexBuffer,
		vertices.data()
	);

	CodeRed::ResourceHelper::updateBuffer(
		mRuntimeSharing->device(),
		mRuntimeSharing->allocator(),
		mAxisIndexBuffer,
		indices.data()
	);

	mIndexCount = indices.size();
	
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
	else LRTR_ERROR("We do not support Vulkan Mode.");
}

void LRTR::CoordinateRenderSystem::update(const StringGroup<std::shared_ptr<Shape>>& shapes, float delta)
{
	auto axisBuffer = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuBuffer>("AxisBuffer");

	std::vector<AxisBufferData> data;

	static auto xAxis = Vector3f(1, 0, 0);
	
	for (const auto& shape : shapes) {
		if (shape.second->hasComponent<CoordinateSystem>()) {
			const auto component = shape.second->component<CoordinateSystem>();

			std::array<AxisBufferData, 3> axes;

			for (size_t index = 0; index < 3; index++) {
				auto axis = static_cast<Axis>(index);

				const auto rAngle = MathUtility::acos(MathUtility::dot(xAxis, component->axis(axis)));
				const auto rAxis = MathUtility::cross(xAxis, component->axis(axis));
				const auto rTransform = rAxis == Vector3f(0) ? Matrix4x4f(1) : Transform::rotate(rAngle, rAxis).matrix();
				const auto sTransform = Transform::scale(Vector3f(10, 1, 1)).matrix();
				const auto tTransform = Transform::translate(Vector3f(0.5f, 0, 0)).matrix();
				
				axes[index].Transform = rTransform * tTransform * sTransform ;
				axes[index].Color = component->color(axis);

				data.push_back(axes[index]);
			}
		}
	}

	//if the axis buffer is not enough to store the data
	//we will create new buffer and reset it
	if (axisBuffer->count() < data.size()) {
		axisBuffer = mDevice->createBuffer(
			CodeRed::ResourceInfo::GroupBuffer(
				sizeof(AxisBufferData), data.size()
			)
		);

		mFrameResources[mCurrentFrameIndex].set("AxisBuffer", axisBuffer);
	}

	mAxisCount = data.size();
	
	CodeRed::ResourceHelper::updateBuffer(axisBuffer, data.data(), sizeof(AxisBufferData) * data.size());
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
	
	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(descriptorHeap);

	commandList->setVertexBuffer(mAxisVertexBuffer);
	commandList->setIndexBuffer(mAxisIndexBuffer);

	commandList->drawIndexed(mIndexCount, mAxisCount);
	
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
