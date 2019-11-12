#include "Scene.hpp"

#include "../Core/Logging.hpp"

LRTR::Scene::Scene(
	const std::string& name,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	mName(name), mDevice(device)
{
	mCommandAllocator = mDevice->createCommandAllocator();
	mCommandList = mDevice->createGraphicsCommandList(mCommandAllocator);
}

auto LRTR::Scene::generate(
	const std::shared_ptr<CodeRed::GpuTexture>& texture,
	const std::shared_ptr<SceneCamera>& camera)
	-> std::shared_ptr<CodeRed::GpuGraphicsCommandList>
{
	setTarget(texture);

	mCommandAllocator->reset();
	mCommandList->beginRecording();

	mCommandList->beginRenderPass(mRenderPass, mFrameBuffer);
	
	mCommandList->endRenderPass();
	
	mCommandList->endRecording();

	return mCommandList;
}

auto LRTR::Scene::cameras() noexcept -> StringGroup<std::shared_ptr<SceneCamera>>& 
{
	return mCameras;
}

auto LRTR::Scene::name() const noexcept -> std::string
{
	return mName;
}

void LRTR::Scene::update(float delta)
{
}

void LRTR::Scene::setTarget(const std::shared_ptr<CodeRed::GpuTexture>& texture)
{
	if (mFrameBuffer != nullptr && mFrameBuffer->renderTarget(0) == texture)
		return;

	LRTR_ERROR_IF(
		texture == nullptr,
		"texture can not be nullptr."
	);
	
	//when we change the render target, we need reset the frame buffer
	//and render pass.
	mFrameBuffer = mDevice->createFrameBuffer(texture);
	mRenderPass = mDevice->createRenderPass(
		CodeRed::Attachment::RenderTarget(
			texture->format(),
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead));

	mRenderPass->setClear(texture->clearValue());
}