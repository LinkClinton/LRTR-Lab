#include "PastEffectRenderSystem.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Graphics/ShaderCompiler.hpp"

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

	mPipelineInfo->setResourceLayout(mResourceLayout);


	if (mDevice->apiVersion() == CodeRed::APIVersion::DirectX12) {
		const auto vShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/.hlsl");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/DirectX12/.hlsl");

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
			"./Resources/Shaders/Systems/Vulkan/.vert");
		const auto fShaderCode = CodeRed::ShaderCompiler::readShader(
			"./Resources/Shaders/Systems/Vulkan/.frag");

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

void LRTR::PastEffectRenderSystem::update(const Group<Identity, std::shared_ptr<Shape>>& shapes, float delta)
{
	mFrameResources[mCurrentFrameIndex].set<CodeRed::GpuTexture>("SkyBox", nullptr);
	
	for (const auto& shape : shapes) {
		if (shape.second->hasComponent<SkyBox>()) {
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

	const auto descriptorHeap = mFrameResources[mCurrentFrameIndex].get<CodeRed::GpuDescriptorHeap>("DescriptorHeap");

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
	const auto viewMatrix = cameraComponent->toScreen().matrix() *
		camera->component<TransformWrap>()->transform().inverseMatrix();

	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, &viewMatrix, sizeof(Matrix4x4f));
}
