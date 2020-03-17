#include "DeferredShadingWorkflow.hpp"

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Scenes/Components/MeshData/TrianglesMesh.hpp"

#include "../Shaders/CompileShaderWorkflow.hpp"

void LRTR::DeferredShadingBuffer::update(
	const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
	const std::shared_ptr<CodeRed::GpuFrameBuffer>& buffer)
{
	// if the size is same, we do not need update the baked buffer
	if (FrameBuffer != nullptr &&
		FrameBuffer->renderTarget()->width() == buffer->renderTarget()->width() &&
		FrameBuffer->renderTarget()->height() == buffer->renderTarget()->height())
		return;

	const auto width = buffer->renderTarget()->width();
	const auto height = buffer->renderTarget()->height();

	BaseColorAndRoughness = device->createTexture(
		CodeRed::ResourceInfo::RenderTarget(
			width, height, CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ClearValue(0, 0, 0, 0)
		)
	);

	PositionAndOcclusion = device->createTexture(
		CodeRed::ResourceInfo::RenderTarget(
			width, height, CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ClearValue(0, 0, 0, 0)
		)
	);
	
	EmissiveAndMetallic = device->createTexture(
		CodeRed::ResourceInfo::RenderTarget(
			width, height, CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ClearValue(0, 0, 0, 0)
		)
	);

	NormalAndBlur = device->createTexture(
		CodeRed::ResourceInfo::RenderTarget(
			width, height, CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ClearValue(0, 0, 0, 0)
		)
	);

	Depth = device->createTexture(
		CodeRed::ResourceInfo::DepthStencil(
			width, height, CodeRed::PixelFormat::Depth32BitFloat,
			CodeRed::ClearValue(1, 0)
		)
	);
	
	FrameBuffer = device->createFrameBuffer(
		{
			BaseColorAndRoughness->reference(),
			PositionAndOcclusion->reference(),
			EmissiveAndMetallic->reference(),
			NormalAndBlur->reference()
		},
		Depth->reference()
	);
}

LRTR::DeferredShadingWorkflow::DeferredShadingWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	mDevice(device)
{
	mSampler = mDevice->createSampler(CodeRed::SamplerInfo(
		16,
		CodeRed::AddressMode::Repeat,
		CodeRed::AddressMode::Repeat, 
		CodeRed::AddressMode::Repeat));

	//resource 0 : material properties
	//resource 1 : transform matrix
	//resource 2 : camera matrix
	//resource 3 : metallic texture
	//resource 4 : baseColor texture
	//resource 5 : roughness texture
	//resource 6 : occlusion texture
	//resource 7 : normalMap texture
	//resource 8 : emissive texture
	//resource 9 : sampler
	//resource 10 : hasBaseColor, HasRoughness, HasOcclusion, HasNormalMap, HasMetallic, HasEmissive, HasBlurred, index
	mResourceLayout = device->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 1),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 2),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 3),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 4),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 5),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 6),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 7),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 8)
		},
		{
			CodeRed::SamplerLayoutElement(mSampler, 0, 1)
		},
		CodeRed::Constant32Bits(8, 0, 2)
	);

	mPipelineInfo = std::make_shared<CodeRed::PipelineInfo>(mDevice);

	const auto pipelineFactory = mPipelineInfo->pipelineFactory();

	mPipelineInfo->setInputAssemblyState(
		pipelineFactory->createInputAssemblyState(
			{
				CodeRed::InputLayoutElement("POSITION", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 0),
				CodeRed::InputLayoutElement("TEXCOORD", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 1),
				CodeRed::InputLayoutElement("TANGENT", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 2),
				CodeRed::InputLayoutElement("NORMAL", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 3)
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

	// because we use 4 frame buffers, so we need two blend properties
	mPipelineInfo->setBlendState(pipelineFactory->createBlendState(4));
	
	mRenderPass = device->createRenderPass(
		{
			CodeRed::Attachment::RenderTarget(
				CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead),

			CodeRed::Attachment::RenderTarget(
				CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead),

			CodeRed::Attachment::RenderTarget(
				CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead),

			CodeRed::Attachment::RenderTarget(
				CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead)
		},
		CodeRed::Attachment::DepthStencil(
			CodeRed::PixelFormat::Depth32BitFloat,
			CodeRed::ResourceLayout::DepthStencil,
			CodeRed::ResourceLayout::GeneralRead,
			CodeRed::AttachmentLoad::Clear,
			CodeRed::AttachmentStore::Store
		)
	);

	mRenderPass->setClear(std::vector<CodeRed::ClearValue>
	{
		CodeRed::ClearValue(0, 0, 0, 0),
		CodeRed::ClearValue(0, 0, 0, 0),
		CodeRed::ClearValue(0, 0, 0, 0),
		CodeRed::ClearValue(0, 0, 0, 0)
	}, CodeRed::ClearValue(1, 0));

	CompileShaderWorkflow workflow;

	const auto sourceLanguage = SourceLanguage::eHLSL;
	const auto targetLanguage = mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		TargetLanguage::eDXIL : TargetLanguage::eSPIRV;

	const auto vShaderFile = "./Resources/Shaders/Workflow/HLSL/DeferredShadingVert.hlsl";
	const auto fShaderFile = "./Resources/Shaders/Workflow/HLSL/DeferredShadingFrag.hlsl";

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
}

auto LRTR::DeferredShadingWorkflow::resourceLayout() const noexcept -> std::shared_ptr<CodeRed::GpuResourceLayout>
{
	return mResourceLayout;
}

auto LRTR::DeferredShadingWorkflow::work(const WorkflowStartup<DeferredShadingWorkflowInput>& startup)
	-> DeferredShadingWorkflowOutput
{
	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		startup.InputData.Sharing->assetManager()->components().at("MeshData"));

	const auto commandList = startup.InputData.CommandList;

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);

	commandList->setVertexBuffers({
			meshDataAssetComponent->positions(),
			meshDataAssetComponent->texCoords(),
			meshDataAssetComponent->tangents(),
			meshDataAssetComponent->normals()
		});
	commandList->setIndexBuffer(meshDataAssetComponent->indices());

	commandList->beginRenderPass(mRenderPass, startup.InputData.DeferredShadingBuffer.FrameBuffer);

	commandList->setViewPort(startup.InputData.DeferredShadingBuffer.FrameBuffer->fullViewPort());
	commandList->setScissorRect(startup.InputData.DeferredShadingBuffer.FrameBuffer->fullScissorRect());
	
	for (size_t index = 0; index < startup.InputData.DrawCalls.size(); index++) {
		const auto drawCall = startup.InputData.DrawCalls[index];
		const auto drawProperty = meshDataAssetComponent->get(drawCall.Mesh);

		commandList->setDescriptorHeap(startup.InputData.DescriptorHeaps[index]);

		commandList->setConstant32Bits({
			drawCall.HasBaseColor,
			drawCall.HasRoughness,
			drawCall.HasOcclusion,
			drawCall.HasNormalMap,
			drawCall.HasMetallic,
			drawCall.HasEmissive,
			drawCall.HasBlurred,
			static_cast<unsigned>(index)
			});

		commandList->drawIndexed(drawProperty.IndexCount, 1,
			drawProperty.StartIndexLocation,
			drawProperty.StartVertexLocation,
			0);
	}

	commandList->endRenderPass();
	
	return {};
}
