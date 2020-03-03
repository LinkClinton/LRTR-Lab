#include "Scene.hpp"

#include "../Core/Logging.hpp"

#include "Components/CollectionLabel.hpp"
#include "Components/CameraGroup.hpp"

#include "Shapes/SceneProperty.hpp"

LRTR::Scene::Scene(
	const std::string& name,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const size_t maxFrameCount) :
	mDevice(device), mMaxFrameCount(maxFrameCount), mName(name)
{
	mCommandAllocators.push_back(mDevice->createCommandAllocator());
	mCommandAllocators.push_back(mDevice->createCommandAllocator());
	
	//first for processing, second for rendering
	mCommandLists.push_back(mDevice->createGraphicsCommandList(mCommandAllocators[0]));
	mCommandLists.push_back(mDevice->createGraphicsCommandList(mCommandAllocators[1]));

	add(mProperty = std::make_shared<SceneProperty>());

	mProperty->component<CollectionLabel>()->set("Collection", "Scene");
}

void LRTR::Scene::add(const std::shared_ptr<Shape>& shape)
{
	if (std::dynamic_pointer_cast<SceneCamera>(shape) != nullptr)
		mProperty->component<CameraGroup>()->addCamera(shape);
	
	mShapes.insert({ shape->identity(), shape });
}

void LRTR::Scene::addSystem(const std::shared_ptr<System>& system)
{
	mSystems.push_back(system);
}

void LRTR::Scene::remove(const Identity& identity)
{
	if (std::dynamic_pointer_cast<SceneCamera>(mShapes.at(identity)) != nullptr)
		mProperty->component<CameraGroup>()->removeCamera(identity);
	
	mShapes.erase(identity);
}

auto LRTR::Scene::name() const noexcept -> std::string
{
	return mName;
}

auto LRTR::Scene::shapes() const noexcept -> const Group<Identity, std::shared_ptr<Shape>>&
{
	return mShapes;
}

auto LRTR::Scene::systems() const noexcept -> const std::vector<std::shared_ptr<System>>& 
{
	return mSystems;
}

auto LRTR::Scene::property() const noexcept -> std::shared_ptr<Shape>
{
	return mProperty;
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

	mCommandAllocators[0]->reset();
	mCommandAllocators[1]->reset();

	mCommandLists[0]->beginRecording();
	mCommandLists[1]->beginRecording();

	mCommandLists[1]->beginRenderPass(mRenderPass, mFrameBuffer);
	mCommandLists[1]->setViewPort(mFrameBuffer->fullViewPort());
	mCommandLists[1]->setScissorRect(mFrameBuffer->fullScissorRect());

	for (const auto& system : mSystems) {
		auto renderSystem = std::dynamic_pointer_cast<RenderSystem>(system);

		if (renderSystem != nullptr)
			renderSystem->render(mCommandLists, mFrameBuffer, camera, delta);
	}
	
	mCommandLists[1]->endRenderPass();

	mCommandLists[0]->endRecording();
	mCommandLists[1]->endRecording();

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mMaxFrameCount;

	return mCommandLists;
}

void LRTR::Scene::setTarget(const std::shared_ptr<CodeRed::GpuTexture>& texture)
{
	if (mFrameBuffer != nullptr && mFrameBuffer->renderTarget(0)->source() == texture)
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

	mFrameBuffer = mDevice->createFrameBuffer({ texture->reference() }, mDepthStencil->reference());
	mRenderPass = mDevice->createRenderPass(
		{
			CodeRed::Attachment::RenderTarget(
			texture->format(),
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead)
		},
		CodeRed::Attachment::DepthStencil(
			mDepthStencil->format(),
			CodeRed::ResourceLayout::DepthStencil,
			CodeRed::ResourceLayout::GeneralRead
		));

	mRenderPass->setClear(texture->clearValue(), mDepthStencil->clearValue());
}
