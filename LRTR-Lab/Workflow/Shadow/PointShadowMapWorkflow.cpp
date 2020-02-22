#include "PointShadowMapWorkflow.hpp"

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"

#include "../Shaders/CompileShaderWorkflow.hpp"

#include "../../Shared/Transform.hpp"

LRTR::PointShadowMapWorkflow::PointShadowMapWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	mDevice(device)
{
	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(sizeof(Matrix4x4f) * 8));

	// resource 0 : view buffer for camera
	// resource 1 : transform buffer for objects
	// resource 2 : faceIndex, transformIndex, farPlane, lightPositionX, lightPositionY, lightPositionZ
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 0, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::GroupBuffer, 1, 0)
		},
		{},
		CodeRed::Constant32Bits(6, 0, 2));

	mDescriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);

	mPipelineInfo = std::make_shared<CodeRed::PipelineInfo>(mDevice);

	const auto pipelineFactory = mPipelineInfo->pipelineFactory();

	mPipelineInfo->setInputAssemblyState(
		pipelineFactory->createInputAssemblyState(
			{
				CodeRed::InputLayoutElement("POSITION", CodeRed::PixelFormat::RedGreenBlue32BitFloat, 0)
			},
			CodeRed::PrimitiveTopology::TriangleList
		)
	);

	mPipelineInfo->setResourceLayout(mResourceLayout);

	mRenderPass = mDevice->createRenderPass(std::nullopt, 
		CodeRed::Attachment::DepthStencil(CodeRed::PixelFormat::Depth32BitFloat));

	CompileShaderWorkflow workflow;

	const auto sourceLanguage = SourceLanguage::eHLSL;
	const auto targetLanguage = mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		TargetLanguage::eDXIL : TargetLanguage::eSPIRV;

	const auto vShaderFile = "./Resources/Shaders/Workflow/HLSL/PointShadowMapVert.hlsl";
	const auto fShaderFile = "./Resources/Shaders/Workflow/HLSL/PointShadowMapFrag.hlsl";
	
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

auto LRTR::PointShadowMapWorkflow::work(const WorkflowStartup<PointShadowMapInput>& startup) -> PointShadowMapOutput
{
	assert(startup.InputData.ShadowMap->width() == startup.InputData.ShadowMap->height());
	
	const auto position = startup.InputData.Position;
	
	Matrix4x4f views[8] = {
		Transform::lookAt(Vector3f(position), position + Vector3f(+1.f, +0.f, +0.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
		Transform::lookAt(Vector3f(position), position + Vector3f(-1.f, +0.f, +0.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
		Transform::lookAt(Vector3f(position), position + Vector3f(+0.f, +1.f, +0.f), Vector3f(+0.f, +0.f, +1.f)).matrix(),
		Transform::lookAt(Vector3f(position), position + Vector3f(+0.f, -1.f, +0.f), Vector3f(+0.f, +0.f, -1.f)).matrix(),
		Transform::lookAt(Vector3f(position), position + Vector3f(+0.f, +0.f, +1.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
		Transform::lookAt(Vector3f(position), position + Vector3f(+0.f, +0.f, -1.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
	};

	for (size_t index = 0; index < 6; index++)
		views[index] = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, startup.InputData.Radius) * views[index];

	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, &views, sizeof(views));

	mDescriptorHeap->bindBuffer(mViewBuffer, 0);
	mDescriptorHeap->bindBuffer(startup.InputData.Transform, 1);

	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		startup.InputData.Sharing->assetManager()->components().at("MeshData"));

	const auto commandList = startup.InputData.CommandList;

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(mDescriptorHeap);

	commandList->setVertexBuffers({ meshDataAssetComponent->positions() });
	commandList->setIndexBuffer(meshDataAssetComponent->indices());

	const CodeRed::ViewPort viewPort = {
		0, 0,
		static_cast<float>(startup.InputData.ShadowMap->width()),
		static_cast<float>(startup.InputData.ShadowMap->height()), 0.f, 1.f
	};

	const CodeRed::ScissorRect scissorRect = {
		0, 0,
		startup.InputData.ShadowMap->width(),
		startup.InputData.ShadowMap->height()
	};
	
	for (size_t face = 0; face < 6; face++) {
		if (mFrameBuffer[face] == nullptr) {
			mFrameBuffer[face] = mDevice->createFrameBuffer(nullptr,
				startup.InputData.ShadowMap->reference(CodeRed::TextureRefInfo(
					CodeRed::ValueRange<size_t>(0, 1),
					CodeRed::ValueRange<size_t>(face, face + 1))));
		}else {
			mFrameBuffer[face]->reset(nullptr,
				startup.InputData.ShadowMap->reference(CodeRed::TextureRefInfo(
					CodeRed::ValueRange<size_t>(0, 1),
					CodeRed::ValueRange<size_t>(face, face + 1))));
		}

		commandList->beginRenderPass(mRenderPass, mFrameBuffer[face]);
		
		commandList->setViewPort(viewPort);
		commandList->setScissorRect(scissorRect);

		for (size_t index = 0; index < startup.InputData.Info.size(); index++) {
			const auto drawProperty = meshDataAssetComponent->get(startup.InputData.Info[index].Mesh);

			commandList->setConstant32Bits({
				static_cast<unsigned>(face),
				static_cast<unsigned>(startup.InputData.Info[index].Index),
				startup.InputData.Radius,
				startup.InputData.Position.x,
				startup.InputData.Position.y,
				startup.InputData.Position.z
				});

			commandList->drawIndexed(drawProperty.IndexCount, 1,
				drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);
		}
		
		commandList->endRenderPass();
	}
	
	return { startup.InputData.ShadowMap };
}
