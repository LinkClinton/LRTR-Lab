#include "ImageBasedLightingWorkflow.hpp"

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../Shaders/CompileShaderWorkflow.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Files/FileSystem.hpp"
#include "../../Shared/Math/Math.hpp"
#include "../../Shared/Transform.hpp"
#include "../../Shared/Hash.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include <filesystem>

const static auto PBRCacheLocation = "./Resources/Caches/PBR/";

auto LRTR::ImageBasedLightingInput::string() const noexcept -> std::string
{
	return FileName +
		std::to_string(EnvironmentMapSize) +
		std::to_string(IrradianceMapSize) +
		std::to_string(PreFilteringMapSize) +
		std::to_string(PreFilteringMipLevels) +
		std::to_string(PreComputingBRDFSize);
}

LRTR::ImageBasedLightingWorkflow::ImageBasedLightingWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	mDevice(device)
{
	mAllocator = mDevice->createCommandAllocator();

	mViewBuffer = mDevice->createBuffer(
		CodeRed::ResourceInfo::ConstantBuffer(sizeof(Matrix4x4f) * 8));
	
	mSampler = mDevice->createSampler(CodeRed::SamplerInfo(16));

	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 0, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 1, 0)
		}, {
			CodeRed::SamplerLayoutElement(mSampler, 0, 1)
		}, CodeRed::Constant32Bits(1, 0, 2)
	);

	mDescriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);
	
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

	mPipelineInfo->setDepthStencilState(
		pipelineFactory->createDetphStencilState(false)
	);

	mPipelineInfo->setResourceLayout(mResourceLayout);

	//Build Render Pass
	mEnvironmentMapRenderPass = mDevice->createRenderPass(
		CodeRed::Attachment::RenderTarget(CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead));
	
	//Compile Shaders 
	CompileShaderWorkflow workflow;
	
	const auto EnvironmentMapVShaderFile =
		mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		"./Resources/Shaders/Workflow/DirectX12/EnvironmentMapVert.hlsl" :
		"";
	const auto EnvironmentMapFShaderFile =
		mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		"./Resources/Shaders/Workflow/DirectX12/EnvironmentMapFrag.hlsl" :
		"";

	mEnvironmentMapVShader = pipelineFactory->createShaderState(
		CodeRed::ShaderType::Vertex,
		workflow.start({ CompileShaderInput(
			EnvironmentMapVShaderFile,
			mDevice->apiVersion(),
			CodeRed::ShaderType::Vertex
		) })
	);

	mEnvironmentMapFShader = pipelineFactory->createShaderState(
		CodeRed::ShaderType::Pixel,
		workflow.start({ CompileShaderInput(
			EnvironmentMapFShaderFile,
			mDevice->apiVersion(),
			CodeRed::ShaderType::Pixel
		) })
	);
}

auto LRTR::ImageBasedLightingWorkflow::readCache(const WorkflowStartup<ImageBasedLightingInput>& startup)
	-> std::optional<ImageBasedLightingOutput>
{
	mSha256Key = Hash::sha256(FileSystem::read(startup.InputData.FileName) + startup.InputData.string());

	if (!std::filesystem::exists(PBRCacheLocation + mSha256Key))
		return std::nullopt;

	const auto data = FileSystem::read<CodeRed::Byte>(PBRCacheLocation + mSha256Key);

	IBLOutput output;

	output.EnvironmentMap = mDevice->createTexture(
		CodeRed::ResourceInfo::CubeMap(
			startup.InputData.EnvironmentMapSize,
			startup.InputData.EnvironmentMapSize,
			CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat
		)
	);

	CodeRed::ResourceHelper::updateTexture(mDevice, mAllocator, startup.InputData.Queue,
		output.EnvironmentMap, data.data());
	
	return output;
}

void LRTR::ImageBasedLightingWorkflow::writeCache(
	const WorkflowStartup<ImageBasedLightingInput>& startup,
	const ImageBasedLightingOutput& output)
{
	//const auto data = CodeRed::ResourceHelper::readTexture(mDevice, mAllocator, startup.InputData.Queue, output.EnvironmentMap);

	//FileSystem::write<CodeRed::Byte>(PBRCacheLocation + mSha256Key, data);
}

auto LRTR::ImageBasedLightingWorkflow::work(
	const WorkflowStartup<ImageBasedLightingInput>& startup) -> ImageBasedLightingOutput
{
	mAllocator->reset();
	
	ImageBasedLightingOutput output;

	// Resource Build and Bind Stage
	const auto hdrTexture = CodeRed::ResourceHelper::loadTexture(
		mDevice, mAllocator, startup.InputData.Queue, startup.InputData.FileName,
		CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat);

	output.EnvironmentMap = mDevice->createTexture(
		CodeRed::ResourceInfo::CubeMap(
			startup.InputData.EnvironmentMapSize,
			startup.InputData.EnvironmentMapSize,
			CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			1,
			CodeRed::ResourceUsage::RenderTarget
		)
	);

	Matrix4x4f views[8] = {
		Transform::lookAt(Vector3f(0), Vector3f(+1.f, +0.f, +0.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
		Transform::lookAt(Vector3f(0), Vector3f(-1.f, +0.f, +0.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
		Transform::lookAt(Vector3f(0), Vector3f(+0.f, +1.f, +0.f), Vector3f(+0.f, +0.f, +1.f)).matrix(),
		Transform::lookAt(Vector3f(0), Vector3f(+0.f, -1.f, +0.f), Vector3f(+0.f, +0.f, -1.f)).matrix(),
		Transform::lookAt(Vector3f(0), Vector3f(+0.f, +0.f, +1.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
		Transform::lookAt(Vector3f(0), Vector3f(+0.f, +0.f, -1.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
	};

	for (size_t index = 0; index < 6; index++)
		views[index] = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.0f) * views[index];

	CodeRed::ResourceHelper::updateBuffer(mViewBuffer, &views, sizeof(views));

	mDescriptorHeap->bindBuffer(mViewBuffer, 0);
	mDescriptorHeap->bindTexture(hdrTexture, 1);

	//Pipeline Build Stage
	mPipelineInfo->setVertexShaderState(mEnvironmentMapVShader);
	mPipelineInfo->setPixelShaderState(mEnvironmentMapFShader);
	mPipelineInfo->setRenderPass(mEnvironmentMapRenderPass);

	mPipelineInfo->updateState();

	//Generate Environment Map
	const auto commandList = mDevice->createGraphicsCommandList(mAllocator);
	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		startup.InputData.mRuntimeSharing->assetManager()->components().at("MeshData"));
	
	commandList->beginRecording();

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(mDescriptorHeap);
	
	commandList->setVertexBuffer(meshDataAssetComponent->positions());
	commandList->setIndexBuffer(meshDataAssetComponent->indices());

	for (size_t index = 0; index < 6; index++) {
		const auto drawProperty = meshDataAssetComponent->get("SkyBox");
		const auto frameBuffer = mDevice->createFrameBuffer(
			output.EnvironmentMap->reference(
				CodeRed::TextureRefInfo(
					CodeRed::ValueRange<size_t>(0, 1),
					CodeRed::ValueRange<size_t>(index, index + 1))));
		
		commandList->beginRenderPass(mEnvironmentMapRenderPass, frameBuffer);

		commandList->setViewPort(frameBuffer->fullViewPort());
		commandList->setScissorRect(frameBuffer->fullScissorRect());
		commandList->setConstant32Bits({ static_cast<unsigned>(index) });

		commandList->drawIndexed(drawProperty.IndexCount, 1,
			drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);

		commandList->endRenderPass();
	}
	
	commandList->endRecording();
	
	startup.InputData.Queue->execute({ commandList });
	startup.InputData.Queue->waitIdle();
	
	return output;
}
