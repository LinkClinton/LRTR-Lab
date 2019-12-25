#pragma once

#include <CodeRed/Shared/Enum/APIVersion.hpp>
#include <CodeRed/Shared/Enum/ShaderType.hpp>

#include "../Workflow.hpp"

#include <string>
#include <vector>

namespace LRTR {

	struct CompileShaderInput {
		std::string FileName;
		CodeRed::APIVersion Version = CodeRed::APIVersion::DirectX12;
		CodeRed::ShaderType Type = CodeRed::ShaderType::Vertex;

		CompileShaderInput() = default;

		CompileShaderInput(const std::string& fileName,
			const CodeRed::APIVersion version,
			const CodeRed::ShaderType type) :
			FileName(fileName), Version(version), Type(type) {}
	};
	
	class CompileShaderWorkflow : public Workflow<CompileShaderInput, std::vector<CodeRed::Byte>> {
	public:
		CompileShaderWorkflow() = default;

		~CompileShaderWorkflow() = default;
	protected:
		auto readCache(const WorkflowStartup<CompileShaderInput>& startup)
			-> std::optional<std::vector<unsigned char>> override;

		void writeCache(
			const WorkflowStartup<CompileShaderInput>& startup,
			const std::vector<unsigned char>& output) override;

		auto work(const WorkflowStartup<CompileShaderInput>& startup) -> std::vector<unsigned char> override;
	private:
		std::string mShaderCode;
		std::string mSha256Key;
	};
	
}