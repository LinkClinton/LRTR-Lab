#include "PostEffectRenderSystem.hpp"

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"

#include "../../Workflow/Shaders/CompileShaderWorkflow.hpp"

#include "../Components/Environment/SkyBox.hpp"


LRTR::PostEffectRenderSystem::PostEffectRenderSystem(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	size_t maxFrameCount) : RenderSystem(sharing, device, maxFrameCount)
{
	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(
			sizeof(Matrix4x4f) * 4
		)
	);

	mSampler = mDevice->createSampler(
		CodeRed::SamplerInfo(16)
	);

	// resource 0 : view buffer
	// resource 1 : cube map for sky box
	// resource 2 : blur texture
	// resource 3 : sampler for sampling
	// resource 4 : isSkyBox, isBlur, isHDR
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer,0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 1),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 2)
		}, {
			CodeRed::SamplerLayoutElement(mSampler, 0, 1)
		}, CodeRed::Constant32Bits(3, 0, 2));

	for (auto& frameResource : mFrameResources) {
		auto descriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);
		
		descriptorHeap->bindBuffer(mViewBuffer, 0);

		frameResource.set("DescriptorHeap", descriptorHeap);
		frameResource.set<CodeRed::GpuTexture>("SkyBox", nullptr);
	}

	mPipelineInfo = std::make_shared<CodeRed::PipelineInfo>(mDevice);

	const auto pipelineFactory = mPipelineInfo->pipelineFactory();

	mPipelineInfo->setInputAssemblyState(
		pipelineFactory->createInputAssemblyState(
			{
				CodeRed::InputLayoutElement("POSITION", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 0),
				CodeRed::InputLayoutElement("TEXCOORD", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 1)
			},
			CodeRed::PrimitiveTopology::TriangleList
		)
	);

	mPipelineInfo->setRasterizationState(
		pipelineFactory->createRasterizationState(
			CodeRed::FrontFace::Clockwise, CodeRed::CullMode::None)
	);

	mPipelineInfo->setBlendState(
		pipelineFactory->createBlendState(
			{
				CodeRed::BlendProperty(
					true, 
					CodeRed::BlendOperator::Add,
					CodeRed::BlendOperator::Add,
					CodeRed::BlendFactor::InvSrcAlpha,
					CodeRed::BlendFactor::InvSrcAlpha,
					CodeRed::BlendFactor::SrcAlpha,
					CodeRed::BlendFactor::SrcAlpha
				)
			}
		)
	);

	mPipelineInfo->setResourceLayout(mResourceLayout);

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
		"./Resources/Shaders/Systems/DirectX12/PostEffectRenderSystemVert.hlsl" :
		"./Resources/Shaders/Systems/Vulkan/PostEffectRenderSystemVert.vert";

	const auto fShaderFile =
		sourceLanguage == SourceLanguage::eHLSL ?
		"./Resources/Shaders/Systems/DirectX12/PostEffectRenderSystemFrag.hlsl" :
		"./Resources/Shaders/Systems/Vulkan/PostEffectRenderSystemFrag.frag";

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

	mGaussianBlurWorkflow = std::make_shared<GaussianBlurWorkflow>(mDevice);
}

void LRTR::PostEffectRenderSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	mFrameResources[mCurrentFrameIndex].set<CodeRed::GpuTexture>("SkyBox", nullptr);
	
	for (const auto& shape : shapes) {
		if (shape.second->hasComponent<SkyBox>() && shape.second->component<SkyBox>()->IsRendered) {
			mFrameResources[mCurrentFrameIndex].set("SkyBox",
				shape.second->component<SkyBox>()->cubeMap());
		}
	}

	if (mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuTexture>("SkyBox") != nullptr) {
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap")
			->bindTexture(mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuTexture>("SkyBox")
				->reference(CodeRed::TextureRefUsage::CubeMap), 1);
	}
}

void LRTR::PostEffectRenderSystem::render(
	const std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>& commandLists,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer, 
	const std::shared_ptr<SceneCamera>& camera,
	float delta)
{
	updatePipeline(frameBuffer);
	updateCamera(camera);

	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		mRuntimeSharing->assetManager()->components().at("MeshData"));
	
	const auto descriptorHeap = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap");
	const auto skyBox = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuTexture>("SkyBox");
	const auto isHDR = skyBox == nullptr ? 0 : CodeRed::PixelFormatSizeOf::get(skyBox->format()) > 4;

	const auto commandList = commandLists[1];

	// because we need render some things to other frame buffer after we finish rendering objects
	// so we will end the render pass and when we finish the rendering
	// we will begin a new render pass with same frame buffer and with render pass that load old pixels
	commandList->endRenderPass();

	mGaussianBlurWorkflow->start({ GaussianBlurInput(
		commandList,
		mBlurFrameBuffer[0], mBlurFrameBuffer[1], mRuntimeSharing) });

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(descriptorHeap);

	commandList->setVertexBuffers({ meshDataAssetComponent->positions(), meshDataAssetComponent->texCoords() });
	commandList->setIndexBuffer(meshDataAssetComponent->indices());
	
	commandList->beginRenderPass(mRenderPass, frameBuffer);

	commandList->setViewPort(frameBuffer->fullViewPort());
	commandList->setScissorRect(frameBuffer->fullScissorRect());
	
	//just render when the sky box is existed
	if (skyBox != nullptr) {
		const auto drawProperty = meshDataAssetComponent->get("SkyBox");

		commandList->setConstant32Bits({ 1, 0, isHDR });

		commandList->drawIndexed(drawProperty.IndexCount, 1,
			drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);
	}
	
	{
		const auto drawProperty = meshDataAssetComponent->get("Quad");

		commandList->setConstant32Bits({ 0, 1, 0 });

		commandList->drawIndexed(drawProperty.IndexCount, 1,
			drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);
	}

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameResources.size();
}

auto LRTR::PostEffectRenderSystem::typeName() const noexcept -> std::string
{
	return "PostEffectRenderSystem";
}

auto LRTR::PostEffectRenderSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(PostEffectRenderSystem);
}

void LRTR::PostEffectRenderSystem::updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer)
{
	if (mRenderPass != nullptr && mRenderPass->compatible(frameBuffer) && 
		mPipelineInfo->graphicsPipeline() != nullptr)
		return;

	std::vector<CodeRed::Attachment> colorAttachments = {};
	std::optional<CodeRed::Attachment> depthAttachment = std::nullopt;

	for (size_t index = 0; index < frameBuffer->size(); index++)
		colorAttachments.push_back(CodeRed::Attachment::RenderTarget(frameBuffer->renderTarget(index)->format(),
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead,
			CodeRed::AttachmentLoad::Load,
			CodeRed::AttachmentStore::Store));

	if (frameBuffer->depthStencil() != nullptr)
		depthAttachment = std::optional<CodeRed::Attachment>(
			CodeRed::Attachment::DepthStencil(frameBuffer->depthStencil()->format(),
				CodeRed::ResourceLayout::DepthStencil,
				CodeRed::ResourceLayout::GeneralRead,
				CodeRed::AttachmentLoad::Load,
				CodeRed::AttachmentStore::Store));

	mRenderPass = mDevice->createRenderPass(colorAttachments, depthAttachment);

	mBlurTexture = mDevice->createTexture(
		CodeRed::ResourceInfo::RenderTarget(
			frameBuffer->renderTarget(1)->width(),
			frameBuffer->renderTarget(1)->height(),
			frameBuffer->renderTarget(1)->format(),
			CodeRed::ClearValue(0, 0, 0, 0)
		)
	);

	mBlurFrameBuffer[0] = mDevice->createFrameBuffer({ mBlurTexture->reference() });
	mBlurFrameBuffer[1] = mDevice->createFrameBuffer({ frameBuffer->renderTarget(1) });

	// update the texture we used
	for (auto& frameResource : mFrameResources) 
		frameResource.get<CodeRed::GpuDescriptorHeap>("DescriptorHeap")->bindTexture(mBlurTexture, 2);
	
	mPipelineInfo->setRenderPass(mRenderPass);
	mPipelineInfo->updateState();
}

void LRTR::PostEffectRenderSystem::updateCamera(const std::shared_ptr<SceneCamera>& camera) const
{
	if (camera == nullptr) return;

	const auto cameraComponent = camera->component<Projective>();
	//just remove the translation of camera
	const auto viewMatrix = cameraComponent->toScreen().matrix() *
		glm::mat4x4(glm::mat3x3(camera->component<TransformWrap>()->transform().inverseMatrix()));

	Matrix4x4f views[4] = {
		viewMatrix,
		Transform::ortho(-1.f, 1.f, 1.f, -1.f, 0.f, 1000.0f).matrix(),
		Matrix4x4f(0),
		Matrix4x4f(0)
	};
	
	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, views, sizeof(Matrix4x4f) * 4);
}
