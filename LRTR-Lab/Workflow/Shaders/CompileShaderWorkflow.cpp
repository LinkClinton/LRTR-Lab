#include "CompileShaderWorkflow.hpp"

#include "../../Shared/Graphics/ShaderCompiler.hpp"
#include "../../Shared/Files/FileSystem.hpp"
#include "../../Shared/Hash.hpp"

#include <filesystem>

const static auto shaderCacheLocation = "./Resources/Caches/Shaders/";

auto LRTR::CompileShaderWorkflow::readCache(
	const WorkflowStartup<CompileShaderInput>& startup) -> std::optional<std::vector<unsigned char>>
{
	mSha256Key = Hash::sha256(mShaderCode = FileSystem::read(startup.InputData.FileName));

	if (!std::filesystem::exists(shaderCacheLocation + mSha256Key))
		return std::nullopt;

	return FileSystem::read<CodeRed::Byte>(shaderCacheLocation + mSha256Key);
}

void LRTR::CompileShaderWorkflow::writeCache(
	const WorkflowStartup<CompileShaderInput>& startup,
	const std::vector<unsigned char>& output)
{
	FileSystem::write(shaderCacheLocation + mSha256Key, output);
}

auto LRTR::CompileShaderWorkflow::work(
	const WorkflowStartup<CompileShaderInput>& startup) -> std::vector<unsigned char>
{
	if (startup.InputData.Version == CodeRed::APIVersion::DirectX12)
		return CodeRed::ShaderCompiler::compileToCso(startup.InputData.Type, mShaderCode);
	else
		return CodeRed::ShaderCompiler::compileToSpv(startup.InputData.Type, mShaderCode);
}
