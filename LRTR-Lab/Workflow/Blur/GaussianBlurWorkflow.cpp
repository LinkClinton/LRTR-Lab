#include "GaussianBlurWorkflow.hpp"

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"

#include "../Shaders/CompileShaderWorkflow.hpp"

#include "../../Shared/Transform.hpp"

LRTR::GaussianBlurWorkflow::GaussianBlurWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	mDevice(device)
{
	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(
			sizeof(Matrix4x4f) * 4
		)
	);

	mSampler = mDevice->createSampler(CodeRed::SamplerInfo(16));
	
	// resource 0 : view buffer for camera
	// resource 1 : transform buffer for objects
	// resource 2 : texture of input
	// resource 3 : horizontal, width, height
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 0, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 1, 0)
		},
		{
			CodeRed::SamplerLayoutElement(mSampler, 0, 1)
		},
		CodeRed::Constant32Bits(3, 0, 2));

	mDescriptorHeaps[0] = mDevice->createDescriptorHeap(mResourceLayout);
	mDescriptorHeaps[1] = mDevice->createDescriptorHeap(mResourceLayout);

	mDescriptorHeaps[0]->bindBuffer(mViewBuffer, 0);
	mDescriptorHeaps[1]->bindBuffer(mViewBuffer, 0);
	
	mRenderPass = mDevice->createRenderPass({
		CodeRed::Attachment::RenderTarget(
			CodeRed::PixelFormat::RedGreenBlueAlpha8BitUnknown,
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead
		)
	});

	mRenderPass->setClear(CodeRed::ClearValue(0, 0, 0, 0));
	
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

	mPipelineInfo->setDepthStencilState(
		pipelineFactory->createDetphStencilState(
			false, false, false,
			CodeRed::CompareOperator::LessEqual
		)
	);
	
	CompileShaderWorkflow workflow;

	const auto sourceLanguage = SourceLanguage::eHLSL;
	const auto targetLanguage = mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		TargetLanguage::eDXIL : TargetLanguage::eSPIRV;

	const auto vShaderFile = "./Resources/Shaders/Workflow/HLSL/GaussianBlurVert.hlsl";
	const auto fShaderFile = "./Resources/Shaders/Workflow/HLSL/GaussianBlurFrag.hlsl";

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

auto LRTR::GaussianBlurWorkflow::work(const WorkflowStartup<GaussianBlurInput>& startup) -> GaussianBlurOutput
{	
	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		startup.InputData.Sharing->assetManager()->components().at("MeshData"));

	const auto commandList = startup.InputData.CommandList;

	std::array<std::shared_ptr<CodeRed::GpuFrameBuffer>, 2> frameBuffer = {
		startup.InputData.Output,
		startup.InputData.Input
	};

	const size_t width[2] = { frameBuffer[0]->renderTarget()->width(), frameBuffer[1]->renderTarget()->width() };
	const size_t height[2] = { frameBuffer[0]->renderTarget()->height(), frameBuffer[1]->renderTarget()->height() };
	
	const Matrix4x4f views[4] = {
		Transform::ortho(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1000.0f).matrix()
	};

	const CodeRed::ViewPort viewPort[2] = {
		CodeRed::ViewPort{ 0, 0, static_cast<float>(width[0]), static_cast<float>(height[0]), 0.0f, 1.0f},
		CodeRed::ViewPort{ 0, 0, static_cast<float>(width[1]), static_cast<float>(height[1]), 0.0f, 1.0f}
	};

	const CodeRed::ScissorRect scissorRect[2] = {
		CodeRed::ScissorRect{ 0, 0, width[0], height[0]},
		CodeRed::ScissorRect{ 0, 0, width[1], height[1]}
	};

	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, views, sizeof(Matrix4x4f) * 4);
	
	mDescriptorHeaps[0]->bindTexture(frameBuffer[1]->renderTarget(), 1);
	mDescriptorHeaps[1]->bindTexture(frameBuffer[0]->renderTarget(), 1);

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);

	commandList->setVertexBuffers({ meshDataAssetComponent->positions(), meshDataAssetComponent->texCoords() });
	commandList->setIndexBuffer(meshDataAssetComponent->indices());

	const auto drawProperty = meshDataAssetComponent->get("Quad");
	
	for (size_t index = 0; index < startup.InputData.Times * 2; index++) {
		const auto which = index % 2;

		commandList->setDescriptorHeap(mDescriptorHeaps[which]);

		commandList->beginRenderPass(mRenderPass, frameBuffer[which]);

		commandList->setViewPort(viewPort[which]);
		commandList->setScissorRect(scissorRect[which]);

		commandList->setConstant32Bits({
			static_cast<unsigned>(which),
			static_cast<unsigned>(width[which]),
			static_cast<unsigned>(height[which])
		});
		
		commandList->drawIndexed(drawProperty.IndexCount, 1,
			drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);

		commandList->endRenderPass();
	}

	return {};
}
