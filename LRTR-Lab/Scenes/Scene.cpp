#include "Scene.hpp"

#include "../Core/Logging.hpp"

LRTR::Scene::Scene(
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator,
	const std::shared_ptr<CodeRed::GpuCommandQueue>& queue) :
	mDevice(device), mCommandAllocator(allocator), mCommandQueue(queue)
{
	mCommandList = mDevice->createGraphicsCommandList(mCommandAllocator);
}

void LRTR::Scene::setTarget(const std::shared_ptr<CodeRed::GpuTexture>& texture)
{
	if (mFrameBuffer != nullptr && mFrameBuffer->renderTarget(0) == texture)
		return;

	//when we change the render target, we need reset the frame buffer
	//and render pass.
	mFrameBuffer = mDevice->createFrameBuffer(texture);
	mRenderPass = mDevice->createRenderPass(
		CodeRed::Attachment::RenderTarget(
			texture->format(),
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead));

}

auto LRTR::Scene::generate(const std::shared_ptr<SceneCamera>& camera) -> std::shared_ptr<CodeRed::GpuTexture>
{
	LRTR_ERROR_IF(
		mFrameBuffer == nullptr || 
		mFrameBuffer->renderTarget(0) == nullptr, 
		"the [frame buffer/render target] can not be nullptr.");

	mCommandList->beginRecording();

	mCommandList->beginRenderPass(mRenderPass, mFrameBuffer);
	
	mCommandList->endRenderPass();
	
	mCommandList->endRecording();
	
	mCommandQueue->execute({ mCommandList });
	
	return mFrameBuffer->renderTarget(0);
}

auto LRTR::Scene::cameras() noexcept -> StringGroup<std::shared_ptr<SceneCamera>>& 
{
	return mCameras;
}

void LRTR::Scene::update(float delta)
{
}
