#include "ImGui.hpp"

void ImGui::BeginPropertyTable(const char* name)
{
	Columns(2, name);
	
	Separator();
}

void ImGui::Property(const char* name, const std::function<void()>& contentCall)
{
	AlignTextToFramePadding();

	Text(name); NextColumn();

	contentCall(); NextColumn();
}

void ImGui::EndPropertyTable()
{
	Columns(1);

	Separator();
}
