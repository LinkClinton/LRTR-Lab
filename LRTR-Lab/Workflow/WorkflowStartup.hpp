#pragma once

#include <any>

namespace LRTR {

	template<typename InputType>
	struct WorkflowStartup {
		InputType InputData = InputType();

		WorkflowStartup() = default;
	};
	
}