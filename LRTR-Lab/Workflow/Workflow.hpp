#pragma once

#include "../Core/Noncopyable.hpp"

#include "WorkflowStartup.hpp"

#include <optional>

namespace LRTR {

	template<typename InputType, typename OutputType, bool EnableCache = true>
	class Workflow : public Noncopyable {
	public:
		Workflow() = default;

		~Workflow() = default;
	};

	template<typename InputType, typename OutputType>
	class Workflow<InputType, OutputType, true> : public Noncopyable {
	public:
		auto start(const WorkflowStartup<InputType>& startup) -> OutputType;
	protected:
		virtual auto readCache(const WorkflowStartup<InputType>& startup) -> std::optional<OutputType> = 0;

		virtual void writeCache(const WorkflowStartup<InputType>& startup, const OutputType& output) = 0;

		virtual auto work(const WorkflowStartup<InputType>& startup) -> OutputType = 0;
	};

	template<typename InputType, typename OutputType>
	class Workflow<InputType, OutputType, false> : public Noncopyable {
	public:
		auto start(const WorkflowStartup<InputType>& startup) -> OutputType;
	protected:
		virtual auto work(const WorkflowStartup<InputType>& startup) -> OutputType = 0;
	};

	template <typename InputType, typename OutputType>
	auto Workflow<InputType, OutputType, true>::start(
		const WorkflowStartup<InputType>& startup) -> OutputType
	{
		const auto cache = readCache(startup);

		if (cache.has_value()) return cache.value();

		auto output = work(startup);

		writeCache(startup, output);

		return output;
	}

	template <typename InputType, typename OutputType>
	auto Workflow<InputType, OutputType, false>::start(const WorkflowStartup<InputType>& startup) -> OutputType
	{
		return work(startup);
	}
}
