#include "PointShadowMapWorkflow.hpp"

#include "../../Runtimes/Managers/Asset/Components/MeshDataAssetComponent.hpp"
#include "../../Runtimes/Managers/Asset/AssetManager.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"

#include "../Shaders/CompileShaderWorkflow.hpp"

#include "../../Shared/Transform.hpp"

namespace LRTR {
	
	auto generateViewMatrix(const PointShadowArea& area) -> std::vector<Matrix4x4f>
	{
		const auto position = area.Position;
		
		std::vector<Matrix4x4f> views = {
			Transform::lookAt(Vector3f(position), position + Vector3f(+1.f, +0.f, +0.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
			Transform::lookAt(Vector3f(position), position + Vector3f(-1.f, +0.f, +0.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
			Transform::lookAt(Vector3f(position), position + Vector3f(+0.f, +1.f, +0.f), Vector3f(+0.f, +0.f, +1.f)).matrix(),
			Transform::lookAt(Vector3f(position), position + Vector3f(+0.f, -1.f, +0.f), Vector3f(+0.f, +0.f, -1.f)).matrix(),
			Transform::lookAt(Vector3f(position), position + Vector3f(+0.f, +0.f, +1.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
			Transform::lookAt(Vector3f(position), position + Vector3f(+0.f, +0.f, -1.f), Vector3f(+0.f, -1.f, +0.f)).matrix(),
			Matrix4x4f(), Matrix4x4f()
		};

		for (size_t index = 0; index < 6; index++)
			views[index] = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, area.Radius) * views[index];

		return views;
	}
	
}

LRTR::PointShadowMapWorkflow::PointShadowMapWorkflow(const std::shared_ptr<CodeRed::GpuLogicalDevice>& device) :
	mDevice(device)
{
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

	mPipelineInfo->setDepthStencilState(
		pipelineFactory->createDetphStencilState(
			true, true, false,
			CodeRed::CompareOperator::LessEqual
		)
	);

	mRenderPass = mDevice->createRenderPass({},
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

	fitDescriptorHeap(startup.InputData.Areas.size());

	const auto meshDataAssetComponent = std::static_pointer_cast<MeshDataAssetComponent>(
		startup.InputData.Sharing->assetManager()->components().at("MeshData"));

	const auto commandList = startup.InputData.CommandList;

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

	for (size_t light = 0; light < startup.InputData.Areas.size(); light++) {
		const auto &area = startup.InputData.Areas[light];
		const auto views = generateViewMatrix(area);

		CodeRed::ResourceHelper::updateBuffer(mViewBuffers[light], views.data(), sizeof(Matrix4x4f) * 8);

		mDescriptorHeaps[light]->bindBuffer(mViewBuffers[light], 0);
		mDescriptorHeaps[light]->bindBuffer(startup.InputData.Transform, 1);

		commandList->setGraphicsPipeline(mPipelineInfo->graphicsPipeline());
		commandList->setResourceLayout(mResourceLayout);

		commandList->setVertexBuffers({ meshDataAssetComponent->positions() });
		commandList->setIndexBuffer(meshDataAssetComponent->indices());
		commandList->setDescriptorHeap(mDescriptorHeaps[light]);

		for (size_t face = 0; face < 6; face++) {
			commandList->beginRenderPass(mRenderPass, area.FrameBuffers[face]);

			commandList->setViewPort(viewPort);
			commandList->setScissorRect(scissorRect);
			
			for (size_t index = 0; index < startup.InputData.Infos.size(); index++) {
				const auto drawProperty = meshDataAssetComponent->get(startup.InputData.Infos[index].Mesh);

				commandList->setConstant32Bits({
					static_cast<unsigned>(face),
					static_cast<unsigned>(startup.InputData.Infos[index].Index),
					area.Radius,
					area.Position.x, area.Position.y, area.Position.z
				});

				commandList->drawIndexed(drawProperty.IndexCount, 1,
					drawProperty.StartIndexLocation, drawProperty.StartVertexLocation);
			}

			commandList->endRenderPass();
		}
	}
	
	return { };
}

void LRTR::PointShadowMapWorkflow::fitDescriptorHeap(const size_t target)
{
	assert(mViewBuffers.size() == mDescriptorHeaps.size());

	if (mViewBuffers.size() >= target && mDescriptorHeaps.size() >= target) return;

	for (size_t index = mViewBuffers.size(); index < target; index++) {
		mViewBuffers.push_back(
			mDevice->createBuffer(
				CodeRed::ResourceInfo::ConstantBuffer(
					sizeof(Matrix4x4f) * 8
				)));
	}

	for (size_t index = mDescriptorHeaps.size(); index < target; index++)
		mDescriptorHeaps.push_back(mDevice->createDescriptorHeap(mResourceLayout));
}
