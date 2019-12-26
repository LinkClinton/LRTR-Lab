#include "ImageBasedLightingWorkflow.hpp"

#include "../../Shared/Graphics/ResourceHelper.hpp"
#include "../../Shared/Files/FileSystem.hpp"
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
	
}
