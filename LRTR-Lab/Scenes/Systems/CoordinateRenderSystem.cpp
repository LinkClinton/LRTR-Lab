#include "CoordinateRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Shared/Graphics/ResourceHelper.hpp"
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
		Vector3f(0, -0.5f, -0.5f),
		Vector3f(0, +0.5f, -0.5f),
		Vector3f(1, +0.5f, -0.5f),
		Vector3f(1, -0.5f, -0.5f),
		Vector3f(0, -0.5f, +0.5f),
		Vector3f(0, +0.5f, +0.5f),
		Vector3f(1, +0.5f, +0.5f),
		Vector3f(1, -0.5f, +0.5f)
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

}

void LRTR::CoordinateRenderSystem::update(const StringGroup<std::shared_ptr<Shape>>& shapes, float delta)
{
	
}

void LRTR::CoordinateRenderSystem::render(
	const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	const StringGroup<std::shared_ptr<Shape>>& shapes, 
	float delta)
{
	
	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameResources.size();
}
