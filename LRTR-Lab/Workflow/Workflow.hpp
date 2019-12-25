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

		auto start(const WorkflowStartup<InputType>& startup) -> OutputType;
	protected:	
		virtual auto readCache(const WorkflowStartup<InputType>& startup) -> std::optional<OutputType> = 0;

		virtual void writeCache(const WorkflowStartup<InputType>& startup, const OutputType& output) = 0;
		
		virtual auto work(const WorkflowStartup<InputType>& startup) -> OutputType = 0;
	};

	template <typename InputType, typename OutputType, bool EnableCache>
	auto Workflow<InputType, OutputType, EnableCache>::start(
		const WorkflowStartup<InputType>& startup) -> OutputType
	{
		//if we do not want to use cache, we will start work
		if (EnableCache == false) return work(startup);

		const auto cache = readCache(startup);

		if (cache.has_value()) return cache.value();

		auto output = work(startup);

		writeCache(startup, output);

		return output;
	}

}
