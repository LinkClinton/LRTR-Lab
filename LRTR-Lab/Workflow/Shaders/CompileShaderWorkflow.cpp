#include "CompileShaderWorkflow.hpp"

#include "../../Shared/Graphics/ShaderCompiler.hpp"
#include "../../Shared/Files/FileSystem.hpp"
#include "../../Shared/Hash.hpp"

#include <Extensions/Compiler/Compiler.hpp>

#include <filesystem>

const static auto shaderCacheLocation = "./Resources/Caches/Shaders/";

auto to_string(const SourceLanguage& source) -> std::string
{
	return source == SourceLanguage::eHLSL ? "HLSL" : "GLSL";
}

auto to_string(const TargetLanguage& target) -> std::string
{
	return target == TargetLanguage::eDXIL ? "DXIL" : "SPIRV";
}

auto LRTR::CompileShaderWorkflow::readCache(
	const WorkflowStartup<CompileShaderInput>& startup) -> std::optional<std::vector<unsigned char>>
{
	mShaderCode = FileSystem::read(startup.InputData.FileName);
	
	mSha256Key = Hash::sha256(mShaderCode +
		to_string(startup.InputData.Source) + "->" +
		to_string(startup.InputData.Target));

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
	const auto option = CompileOption(startup.InputData.Source, startup.InputData.Target, startup.InputData.Type);
	const auto result = compile(mShaderCode, option);

	LRTR_DEBUG_ERROR_IF(result.failed(), result.Message);
	LRTR_DEBUG_WARNING_IF(!result.Message.empty(), result.Message);

	return result.Code;
}
