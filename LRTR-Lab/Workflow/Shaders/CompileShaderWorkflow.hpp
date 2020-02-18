#pragma once

#include <CodeRed/Shared/Enum/APIVersion.hpp>
#include <CodeRed/Shared/Enum/ShaderType.hpp>

#include <Extensions/Compiler/CompileOption.hpp>

#include "../Workflow.hpp"

#include <string>
#include <vector>

using namespace CodeRed::Compiler;

namespace LRTR {

	struct CompileShaderInput {
		std::string FileName;

		CodeRed::ShaderType Type = CodeRed::ShaderType::Vertex;

		SourceLanguage Source = SourceLanguage::eHLSL;
		TargetLanguage Target = TargetLanguage::eDXIL;

		CompileShaderInput() = default;

		CompileShaderInput(
			const std::string& fileName,
			const CodeRed::ShaderType& type,
			const SourceLanguage& source,
			const TargetLanguage& target) :
			FileName(fileName), Type(type), Source(source), Target(target) {}
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