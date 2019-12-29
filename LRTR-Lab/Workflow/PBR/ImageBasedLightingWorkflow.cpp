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

#define IBL_BUILD_ENVIRONMENT_MAP 0
#define IBL_BUILD_IRRADIANCE_MAP 1
#define IBL_BUILD_PRE_FILTERING_MAP 2

auto LRTR::ImageBasedLightingInput::string() const noexcept -> std::string
{
	return FileName +
		std::to_string(EnvironmentMapSize) +
		std::to_string(EnvironmentMipLevels) +
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

	// resource 0 : view buffer with six face
	// resource 1 : hdr texture
	// resource 2 : environment map
	// resource 5 : build type, face index, size of environment map, roughness
	mResourceLayout = mDevice->createResourceLayout(
		{
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Buffer, 0, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 1, 0),
			CodeRed::ResourceLayoutElement(CodeRed::ResourceType::Texture, 2, 0)
		}, {
			CodeRed::SamplerLayoutElement(mSampler, 0, 1)
		}, CodeRed::Constant32Bits(4, 0, 2)
	);

	mDescriptorHeap = mDevice->createDescriptorHeap(mResourceLayout);
	
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

	mPipelineInfo->setDepthStencilState(
		pipelineFactory->createDetphStencilState(false)
	);

	mPipelineInfo->setResourceLayout(mResourceLayout);

	//Build Render Pass
	mRenderPass = mDevice->createRenderPass(
		CodeRed::Attachment::RenderTarget(CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			CodeRed::ResourceLayout::RenderTarget,
			CodeRed::ResourceLayout::GeneralRead));
	
	//Compile Shaders 
	CompileShaderWorkflow workflow;
	
	const auto EnvironmentMapVShaderFile =
		mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		"./Resources/Shaders/Workflow/DirectX12/ImageBasedLightingVert.hlsl" :
		"";
	const auto EnvironmentMapFShaderFile =
		mDevice->apiVersion() == CodeRed::APIVersion::DirectX12 ?
		"./Resources/Shaders/Workflow/DirectX12/ImageBasedLightingFrag.hlsl" :
		"";

	mVertShader = pipelineFactory->createShaderState(
		CodeRed::ShaderType::Vertex,
		workflow.start({ CompileShaderInput(
			EnvironmentMapVShaderFile,
			mDevice->apiVersion(),
			CodeRed::ShaderType::Vertex
		) })
	);

	mFragShader = pipelineFactory->createShaderState(
		CodeRed::ShaderType::Pixel,
		workflow.start({ CompileShaderInput(
			EnvironmentMapFShaderFile,
			mDevice->apiVersion(),
			CodeRed::ShaderType::Pixel
		) })
	);

	mPipelineInfo->setVertexShaderState(mVertShader);
	mPipelineInfo->setPixelShaderState(mFragShader);
	mPipelineInfo->setRenderPass(mRenderPass);

	mPipelineInfo->updateState();
}

auto LRTR::ImageBasedLightingWorkflow::readCache(const WorkflowStartup<ImageBasedLightingInput>& startup)
	-> std::optional<ImageBasedLightingOutput>
{
	mSha256Key = Hash::sha256(FileSystem::read(startup.InputData.FileName) + startup.InputData.string());

	if (!std::filesystem::exists(PBRCacheLocation + mSha256Key))
		return std::nullopt;
	
	IBLOutput output;

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
			startup.InputData.EnvironmentMipLevels,
			CodeRed::ResourceUsage::RenderTarget
		)
	);

	output.IrradianceMap = mDevice->createTexture(
		CodeRed::ResourceInfo::CubeMap(
			startup.InputData.IrradianceMapSize,
			startup.InputData.IrradianceMapSize,
			CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			1,
			CodeRed::ResourceUsage::RenderTarget
		)
	);

	output.PreFilteringMap = mDevice->createTexture(
		CodeRed::ResourceInfo::CubeMap(
			startup.InputData.PreFilteringMapSize,
			startup.InputData.PreFilteringMapSize,
			CodeRed::PixelFormat::RedGreenBlueAlpha32BitFloat,
			startup.InputData.PreFilteringMipLevels,
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
	mDescriptorHeap->bindTexture(output.EnvironmentMap->reference(CodeRed::TextureRefUsage::CubeMap), 2);

	const auto commandList = mDevice->createGraphicsCommandList(mAllocator);
	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		startup.InputData.mRuntimeSharing->assetManager()->components().at("MeshData"));
	
	commandList->beginRecording();

	commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
	commandList->setResourceLayout(mResourceLayout);
	commandList->setDescriptorHeap(mDescriptorHeap);
	
	commandList->setVertexBuffers({ meshDataAssetComponent->positions(), meshDataAssetComponent->texCoords() });
	commandList->setIndexBuffer(meshDataAssetComponent->indices());

	// generate Environment Map with mip levels
	for (size_t arraySlice = 0; arraySlice < 6; arraySlice++) {
		for (size_t mipSlice = 0; mipSlice < startup.InputData.EnvironmentMipLevels; mipSlice++) {
			
			const auto drawProperty = meshDataAssetComponent->get("SkyBox");
			const auto frameBuffer = mDevice->createFrameBuffer(
				output.EnvironmentMap->reference(
					CodeRed::TextureRefInfo(
						CodeRed::ValueRange<size_t>(mipSlice, mipSlice + 1),
						CodeRed::ValueRange<size_t>(arraySlice, arraySlice + 1))));

			commandList->beginRenderPass(mRenderPass, frameBuffer);

			commandList->setViewPort(frameBuffer->fullViewPort());
			commandList->setScissorRect(frameBuffer->fullScissorRect());
			commandList->setConstant32Bits({
				IBL_BUILD_ENVIRONMENT_MAP,
				static_cast<unsigned>(arraySlice),
				static_cast<unsigned>(startup.InputData.EnvironmentMapSize),
				0.0f
				});

			commandList->drawIndexed(drawProperty.IndexCount, 1,
				drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);

			commandList->endRenderPass();
		}
	}

	// generate irradiance map for ambient diffuse light
	for (size_t index = 0; index < 6; index++) {
		const auto drawProperty = meshDataAssetComponent->get("SkyBox");
		const auto frameBuffer = mDevice->createFrameBuffer(
			output.IrradianceMap->reference(
				CodeRed::TextureRefInfo(
					CodeRed::ValueRange<size_t>(0, 1),
					CodeRed::ValueRange<size_t>(index, index + 1))));

		commandList->beginRenderPass(mRenderPass, frameBuffer);

		commandList->setViewPort(frameBuffer->fullViewPort());
		commandList->setScissorRect(frameBuffer->fullScissorRect());
		commandList->setConstant32Bits({
			IBL_BUILD_IRRADIANCE_MAP,
			static_cast<unsigned>(index),
			static_cast<unsigned>(startup.InputData.EnvironmentMapSize),
			0.0f
			});

		commandList->drawIndexed(drawProperty.IndexCount, 1,
			drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);

		commandList->endRenderPass();
	}

	//generate pre-filter map for ambient specular light
	for (size_t arraySlice = 0; arraySlice < 6; arraySlice++) {
		for (size_t mipSlice = 0; mipSlice < startup.InputData.PreFilteringMipLevels; mipSlice++) {

			const auto drawProperty = meshDataAssetComponent->get("SkyBox");
			const auto frameBuffer = mDevice->createFrameBuffer(
				output.PreFilteringMap->reference(
					CodeRed::TextureRefInfo(
						CodeRed::ValueRange<size_t>(mipSlice, mipSlice + 1),
						CodeRed::ValueRange<size_t>(arraySlice, arraySlice + 1))));

			commandList->beginRenderPass(mRenderPass, frameBuffer);

			commandList->setViewPort(frameBuffer->fullViewPort());
			commandList->setScissorRect(frameBuffer->fullScissorRect());
			commandList->setConstant32Bits({
				IBL_BUILD_PRE_FILTERING_MAP,
				static_cast<unsigned>(arraySlice),
				static_cast<unsigned>(startup.InputData.EnvironmentMapSize),
				static_cast<float>(mipSlice) / static_cast<float>(startup.InputData.PreFilteringMipLevels - 1)
				});

			commandList->drawIndexed(drawProperty.IndexCount, 1,
				drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);

			commandList->endRenderPass();
		}
	}
	
	commandList->endRecording();
	
	startup.InputData.Queue->execute({ commandList });
	startup.InputData.Queue->waitIdle();
	
	return output;
}
