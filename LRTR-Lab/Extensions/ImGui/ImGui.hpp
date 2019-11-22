#pragma once

#include <ThirdParties/ImGui/imgui.h>

#include <functional>

namespace ImGui {

	void BeginPropertyTable(const char* name);

	void Property(const char* name, const std::function<void()>& contentCall);

	void AxisProperty(const size_t axisCount, size_t& currentAxis);
	
	void EndPropertyTable();
	
}