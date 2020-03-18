#include "ScreenSpaceAmbientOcclusionWorkflow.hpp"

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Transform.hpp"

#include "../Shaders/CompileShaderWorkflow.hpp"

#include <random>

float ssao_lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void LRTR::ScreenSpaceAmbientOcclusionBuffer::update(
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer)
{
	if (AmbientOcclusion != nullptr &&
		AmbientOcclusion->width() == frameBuffer->renderTarget()->width() &&
		AmbientOcclusion->height() == frameBuffer->renderTarget()->height())
		return;

	const auto width = frameBuffer->renderTarget()->width();
	const auto height = frameBuffer->renderTarget()->height();

	AmbientOcclusionBlurred = device->createTexture(
		CodeRed::ResourceInfo::RenderTarget(
			width, height, CodeRed::PixelFormat::Red32BitFloat,
			CodeRed::ClearValue(0, 0, 0, 0)
		)
	);

	AmbientOcclusion = device->createTexture(
		CodeRed::ResourceInfo::RenderTarget(
			width, height, CodeRed::PixelFormat::Red32BitFloat,
			CodeRed::ClearValue(0, 0, 0, 0)
		)
	);
	
	FrameBufferBlurred = device->createFrameBuffer(
		{
			AmbientOcclusionBlurred->reference()
		}
	);

	FrameBuffer = device->createFrameBuffer(
		{
			AmbientOcclusion->reference()
		}
	);
}

LRTR::ScreenSpaceAmbientOcclusionWorkflow::ScreenSpaceAmbientOcclusionWorkflow(
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) : mDevice(device)
{
	mClampSampler = mDevice->createSampler(CodeRed::SamplerInfo(
		CodeRed::FilterOptions::MinLinearMagLinearMipLinear,
		CodeRed::AddressMode::Clamp,
		CodeRed::AddressMode::Clamp,
		CodeRed::AddressMode::Clamp));

	mSampler = mDevice->createSampler(CodeRed::SamplerInfo(
		CodeRed::FilterOptions::MinLinearMagLinearMipLinear,
		CodeRed::AddressMode::Repeat,
		CodeRed::AddressMode::Repeat,
		CodeRed::AddressMode::Repeat));

	mNoiseTextureBuffer = mDevice->createTextureBuffer(
		CodeRed::TextureBufferInfo(
			4, 4, 1, CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::Dimension::Dimension2D
		)
	);

	mNoiseTexture = mDevice->createTexture(
		CodeRed::ResourceInfo::Texture2D(
			4, 4, CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat
		)
	);

	mSampleBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(
			sizeof(Vector4f) * 64
		)
	);

	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(
			sizeof(Matrix4x4f) * 4
		)
	);

	// resource 0 : view buffer
	// resource 1 : sample buffer
	// resource 2 : noise texture
	// resource 3 : position and occlusion texture
	// resource 4 : view space position texture
	// resource 5 : normal texture
	// resource 6 : ssao texture
	// resource 7 : sampler
	// resource 8 : clamp sampler
	// resource 9 : SampleCount, SampleRadius, SampleBias, NoiseScaleX, NoiseScaleY, Width, Height, IsBlur
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 1),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 2),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 3),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 4),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 5),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 6)
		},
		{
			CodeRed::SamplerLayoutElement(mSampler, 0, 1),
			CodeRed::SamplerLayoutElement(mClampSampler, 1, 1)
		},
		CodeRed::Constant32Bits(8, 0, 2)
	);

	mDescriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);

	mDescriptorHeap->bindBuffer(mViewBuffer, 0);
	mDescriptorHeap->bindBuffer(mSampleBuffer, 1);
	mDescriptorHeap->bindTexture(mNoiseTexture, 2);
	
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

	mPipelineInfo->setResourceLayout(mResourceLayout);

	mPipelineInfo->setRasterizationState(
		pipelineFactory->createRasterizationState(
			CodeRed::FrontFace::CounterClockwise,
			CodeRed::CullMode::None,
			CodeRed::FillMode::Solid
		)
	);

	mPipelineInfo->setDepthStencilState(pipelineFactory->createDetphStencilState(false, false));
	
	mPipelineInfo->setBlendState(pipelineFactory->createBlendState(1));

	mRenderPass = device->createRenderPass(
		{
			CodeRed::Attachment::RenderTarget(
				CodeRed::PixelFormat::Red32BitFloat,
				CodeRed::ResourceLayout::RenderTarget,
				CodeRed::ResourceLayout::GeneralRead)
		}
	);

	mRenderPass->setClear(CodeRed::ClearValue(0, 0, 0, 0));

	CompileShaderWorkflow workflow;

	const auto sourceLanguage = SourceLanguage::eHLSL;
	const auto targetLanguage = mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		TargetLanguage::eDXIL : TargetLanguage::eSPIRV;

	const auto vShaderFile = "./Resources/Shaders/Workflow/HLSL/ScreenSpaceAmbientOcclusionVert.hlsl";
	const auto fShaderFile = "./Resources/Shaders/Workflow/HLSL/ScreenSpaceAmbientOcclusionFrag.hlsl";

	mVertShader = pipelineFactory->createShaderState(
		CodeRed::ShaderType::Vertex,
		workflow.start({ CompileShaderInput(
			vShaderFile,
			CodeRed::ShaderType::Vertex,
			sourceLanguage,
			targetLanguage
		) })
	);

	mFragShader = pipelineFactory->createShaderState(
		CodeRed::ShaderType::Pixel,
		workflow.start({ CompileShaderInput(
			fShaderFile,
			CodeRed::ShaderType::Pixel,
			sourceLanguage,
			targetLanguage
		) })
	);

	mPipelineInfo->setVertexShaderState(mVertShader);
	mPipelineInfo->setPixelShaderState(mFragShader);
	mPipelineInfo->setRenderPass(mRenderPass);

	mPipelineInfo->updateState();

	std::default_random_engine engine;
	std::uniform_real_distribution<float> normal_distribution(0.f, 1.f);
	
	// build the sample kernel and noise vector for SSAO

	// build the noise vector for SSAO, we will build a vector in tangent space to rotate the samples
	for (size_t index = 0; index < mNoiseTexture->width() * mNoiseTexture->height(); index++) {
		mNoiseVectors.push_back(MathUtility::normalize(Vector4f(
			normal_distribution(engine) * 2.0f - 1.0f,
			normal_distribution(engine) * 2.0f - 1.0f,
			0.0f,
			0.0f
		)));
	}

	// build the sample kernel
	for (size_t index = 0; index < 64; index++) {
		auto sample = MathUtility::normalize(Vector4f(
			normal_distribution(engine) * 2.0f - 1.0f,
			normal_distribution(engine) * 2.0f - 1.0f,
			normal_distribution(engine),
			0.0f
		)) * normal_distribution(engine);

		auto scale = static_cast<float>(index) / 64;

		sample = sample * ssao_lerp(0.1f, 1.0f, scale * scale);

		mSamples.push_back(sample);
	}

	mNoiseTextureBuffer->write(mNoiseVectors.data());
	
	CodeRed::ResourceHelper::updateBuffer(mSampleBuffer, mSamples.data(), mSamples.size() * sizeof(Vector4f));
}

auto LRTR::ScreenSpaceAmbientOcclusionWorkflow::work(const WorkflowStartup<ScreenSpaceAmbientOcclusionInput>& startup)
	-> ScreenSpaceAmbientOcclusionOutput
{
	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		startup.InputData.Sharing->assetManager()->components().at("MeshData"));

	const auto commandList = startup.InputData.CommandList;

	mDescriptorHeap->bindTexture(startup.InputData.DSBuffer.PositionAndOcclusion, 3);
	mDescriptorHeap->bindTexture(startup.InputData.DSBuffer.ViewSpacePosition, 4);
	mDescriptorHeap->bindTexture(startup.InputData.DSBuffer.NormalAndBlur, 5);
	mDescriptorHeap->bindTexture(startup.InputData.SSAOBuffer.AmbientOcclusion, 6);

	Matrix4x4f views[4] = {
		Transform::ortho(-1.f, 1.f, 1.f, -1.f, 0.f, 1000.0f).matrix(),
		startup.InputData.ProjectionMatrix,
		startup.InputData.ViewMatrix,
		glm::transpose(glm::inverse(startup.InputData.ViewMatrix))
	};
	
	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, views, sizeof(Matrix4x4f) * 4);
	
	// because we update the noise as texture, we need use command list to update data
	commandList->layoutTransition(mNoiseTextureBuffer, CodeRed::ResourceLayout::CopySource);
	commandList->layoutTransition(mNoiseTexture, CodeRed::ResourceLayout::CopyDestination);

	commandList->copyBufferToTexture(
		CodeRed::TextureBufferCopyInfo(mNoiseTextureBuffer),
		CodeRed::TextureCopyInfo(mNoiseTexture),
		mNoiseTexture->width(), mNoiseTexture->height(), 1
	);
	
	commandList->layoutTransition(mNoiseTextureBuffer, CodeRed::ResourceLayout::GeneralRead);
	commandList->layoutTransition(mNoiseTexture, CodeRed::ResourceLayout::GeneralRead);

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);

	commandList->setVertexBuffers({
			meshDataAssetComponent->positions(),
			meshDataAssetComponent->texCoords()
		});
	commandList->setIndexBuffer(meshDataAssetComponent->indices());

	commandList->setDescriptorHeap(mDescriptorHeap);

	const std::shared_ptr<CodeRed::GpuFrameBuffer> frameBuffers[2] = {
		startup.InputData.SSAOBuffer.FrameBuffer,
		startup.InputData.SSAOBuffer.FrameBufferBlurred
	};
	
	const auto drawProperty = meshDataAssetComponent->get("Quad");

	for (size_t index = 0; index < 1; index++) {
		commandList->beginRenderPass(mRenderPass, frameBuffers[index]);

		commandList->setViewPort(frameBuffers[index]->fullViewPort());
		commandList->setScissorRect(frameBuffers[index]->fullScissorRect());

		// SampleCount, SampleRadius, SampleBias, NoiseScaleX, NoiseScaleY, Width, Height, IsBlur
		commandList->setConstant32Bits({
			64,
			0.5f,
			0.025f,
			static_cast<float>(startup.InputData.SSAOBuffer.AmbientOcclusion->width()) / 4,
			static_cast<float>(startup.InputData.SSAOBuffer.AmbientOcclusion->height()) / 4,
			static_cast<unsigned>(startup.InputData.SSAOBuffer.AmbientOcclusion->width()),
			static_cast<unsigned>(startup.InputData.SSAOBuffer.AmbientOcclusion->height()),
			static_cast<unsigned>(index)
			});

		commandList->drawIndexed(drawProperty.IndexCount, 1,
			drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);
		
		commandList->endRenderPass();
	}
	
	return {};
}
