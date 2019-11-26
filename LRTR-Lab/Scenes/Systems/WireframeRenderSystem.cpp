#include "WireframeRenderSystem.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../../Scenes/Components/TrianglesMesh/TrianglesMesh.hpp"
#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"
#include "../../Shared/Color.hpp"

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
	}
}

void LRTR::WireframeRenderSystem::update(const StringGroup<std::shared_ptr<Shape>>& shapes, float delta)
{
	
}

void LRTR::WireframeRenderSystem::render(
	const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
{
	
}
