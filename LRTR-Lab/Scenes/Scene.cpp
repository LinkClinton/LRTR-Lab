#include "Scene.hpp"

#include "../Core/Logging.hpp"

#include "Components/CameraGroup.hpp"

#include "Shapes/SceneProperty.hpp"

LRTR::Scene::Scene(
	const std::string& name,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const size_t maxFrameCount) :
	mMaxFrameCount(maxFrameCount), mName(name), mDevice(device)
{
	mCommandAllocator = mDevice->createCommandAllocator();
	mCommandList = mDevice->createGraphicsCommandList(mCommandAllocator);

	add("Scene", mProperty = std::make_shared<SceneProperty>());
}

void LRTR::Scene::add(const std::string& name, const std::shared_ptr<Shape>& shape)
{
	if (std::dynamic_pointer_cast<SceneCamera>(shape) != nullptr)
		mProperty->component<CameraGroup>()->addCamera(name);
	
	mShapes.insert({ name, shape });
}

void LRTR::Scene::addSystem(const std::shared_ptr<System>& system)
{
	mSystems.push_back(system);
}

void LRTR::Scene::remove(const std::string& name)
{
	if (std::dynamic_pointer_cast<SceneCamera>(mShapes[name]) != nullptr)
		mProperty->component<CameraGroup>()->removeCamera(name);
	
	mShapes.erase(name);
}

auto LRTR::Scene::name() const noexcept -> std::string
{
	return mName;
}

auto LRTR::Scene::shapes() const noexcept -> const StringGroup<std::shared_ptr<Shape>>& 
{
	return mShapes;
}

auto LRTR::Scene::systems() const noexcept -> const std::vector<std::shared_ptr<System>>& 
{
	return mSystems;
}

auto LRTR::Scene::currentFrameIndex() const noexcept -> size_t
{
	return mCurrentFrameIndex;
}

void LRTR::Scene::update(float delta)
{
	for (const auto& system : mSystems) {
		auto updateSystem = std::dynamic_pointer_cast<UpdateSystem>(system);

		if (updateSystem != nullptr)
			updateSystem->update(mShapes, delta);
	}
}

auto LRTR::Scene::render(
	const std::shared_ptr<CodeRed::GpuTexture>& texture,
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
	-> std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>
{
	setTarget(texture);

	mCommandAllocator->reset();
	mCommandList->beginRecording();

	mCommandList->beginRenderPass(mRenderPass, mFrameBuffer);
	mCommandList->setViewPort(mFrameBuffer->fullViewPort());
	mCommandList->setScissorRect(mFrameBuffer->fullScissorRect());

	for (const auto& system : mSystems) {
		auto renderSystem = std::dynamic_pointer_cast<RenderSystem>(system);

		if (renderSystem != nullptr)
			renderSystem->render(mCommandList, mFrameBuffer, camera, delta);
	}
	
	mCommandList->endRenderPass();

	mCommandList->endRecording();

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mMaxFrameCount;

	return { mCommandList };
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
	mDepthStencil = mDevice->createTexture(
		CodeRed::ResourceInfo::DepthStencil(
			texture->width(),
			texture->height(),
			CodeRed::PixelFormat::Depth32BitFloat
		)
	);
	
	mFrameBuffer = mDevice->createFrameBuffer(texture, mDepthStencil);
	mRenderPass = mDevice->createRenderPass(
		CodeRed::Attachment::RenderTarget(
			texture->format(),
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead),
		CodeRed::Attachment::DepthStencil(
			mDepthStencil->format(),
			CodeRed::ResourceLayout::DepthStencil,
			CodeRed::ResourceLayout::GeneralRead
		));

	mRenderPass->setClear(texture->clearValue(), mDepthStencil->clearValue());
}
