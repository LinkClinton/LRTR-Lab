#include "PastEffectRenderSystem.hpp"

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"

#include "../../Workflow/Shaders/CompileShaderWorkflow.hpp"

#include "../Components/Environment/SkyBox.hpp"


LRTR::PastEffectRenderSystem::PastEffectRenderSystem(
	const std::shared_ptr<RuntimeSharing>& sharing,
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	size_t maxFrameCount) : RenderSystem(sharing, device, maxFrameCount)
{
	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(
			sizeof(Matrix4x4f)
		)
	);

	mSampler = mDevice->createSampler(
		CodeRed::SamplerInfo(16)
	);

	// resource 0 : view buffer
	// resource 1 : cube map for sky box
	// resource 2 : sampler for sampling
	// resource 3 : isSkyBox
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer,0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 1)
		}, {
			CodeRed::SamplerLayoutElement(mSampler, 2)
		}, CodeRed::Constant32Bits(1, 3));

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
				CodeRed::InputLayoutElement("POSITION", CodeRed::PixelFormat::RedGreenBlue32BitFloat)
			},
			CodeRed::PrimitiveTopology::TriangleList
		)
	);

	mPipelineInfo->setRasterizationState(
		pipelineFactory->createRasterizationState(
			CodeRed::FrontFace::Clockwise, CodeRed::CullMode::None)
	);

	mPipelineInfo->setResourceLayout(mResourceLayout);

	CompileShaderWorkflow workflow;

	const auto vShaderFile =
		mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		"./Resources/Shaders/Systems/DirectX12/PastEffectRenderSystemVert.hlsl" :
		"./Resources/Shaders/Systems/Vulkan/.vert";

	const auto fShaderFile =
		mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		"./Resources/Shaders/Systems/DirectX12/PastEffectRenderSystemFrag.hlsl" :
		"./Resources/Shaders/Systems/Vulkan/.frag";

	mPipelineInfo->setVertexShaderState(
		pipelineFactory->createShaderState(
			CodeRed::ShaderType::Vertex,
			workflow.start({ CompileShaderInput(
				vShaderFile,
				mDevice->apiVersion(),
				CodeRed::ShaderType::Vertex
			) })
		)
	);

	mPipelineInfo->setPixelShaderState(
		pipelineFactory->createShaderState(
			CodeRed::ShaderType::Pixel,
			workflow.start({ CompileShaderInput(
				fShaderFile,
				mDevice->apiVersion(),
				CodeRed::ShaderType::Pixel
			) })
		)
	);
}

void LRTR::PastEffectRenderSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	mFrameResources[mCurrentFrameIndex].set<CodeRed::GpuTexture>("SkyBox", nullptr);
	
	for (const auto& shape : shapes) {
		if (shape.second->hasComponent<SkyBox>() && shape.second->component<SkyBox>()->visibility()) {
			mFrameResources[mCurrentFrameIndex].set("SkyBox",
				shape.second->component<SkyBox>()->cubeMap());
		}
	}

	if (mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuTexture>("SkyBox") != nullptr) {
		mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap")
			->bindTexture(mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuTexture>("SkyBox"), 1);
	}
}

void LRTR::PastEffectRenderSystem::render(
	const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
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

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(descriptorHeap);

	commandList->setVertexBuffer(meshDataAssetComponent->positions());
	commandList->setIndexBuffer(meshDataAssetComponent->indices());

	commandList->setConstant32Bits({ 1 });

	//just render when the sky box is existed
	if (skyBox != nullptr) {
		const auto drawProperty = meshDataAssetComponent->get("SkyBox");
		
		commandList->drawIndexed(drawProperty.IndexCount, 1,
			drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);
	}
	
	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameResources.size();
}

auto LRTR::PastEffectRenderSystem::typeName() const noexcept -> std::string
{
	return "PastEffectRenderSystem";
}

auto LRTR::PastEffectRenderSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(PastEffectRenderSystem);
}

void LRTR::PastEffectRenderSystem::updatePipeline(const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer) const
{
	if (CodeRed::PipelineInfo::isCompatible(mPipelineInfo->renderPass(), frameBuffer) &&
		mPipelineInfo->graphicsPipeline() != nullptr) return;

	mPipelineInfo->setRenderPass(frameBuffer);
	mPipelineInfo->updateState();
}

void LRTR::PastEffectRenderSystem::updateCamera(const std::shared_ptr<SceneCamera>& camera) const
{
	if (camera == nullptr) return;

	const auto cameraComponent = camera->component<Projective>();
	//just remove the translation of camera
	const auto viewMatrix = cameraComponent->toScreen().matrix() *
		glm::mat4x4(glm::mat3x3(camera->component<TransformWrap>()->transform().inverseMatrix()));

	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, &viewMatrix, sizeof(Matrix4x4f));
}
