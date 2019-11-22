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

	AlignTextToFramePadding();
	contentCall(); NextColumn();
}

void ImGui::AxisProperty(const size_t axisCount, size_t& currentAxis)
{
	assert(axisCount <= 3);
	
	static const char* AxesName[] = {
		"X",
		"Y",
		"Z"
	};

	BeginPropertyTable("Combo");
	Property("Axis", [&]()
		{
			if (BeginCombo("##Axis", AxesName[currentAxis])) {
				for (size_t index = 0; index < axisCount; index++) {
					const auto selected = (currentAxis == index);

					if (Selectable(AxesName[index], selected))
						currentAxis = index;
					if (selected) SetItemDefaultFocus();
				}
				EndCombo();
			}
		});
}

void ImGui::EndPropertyTable()
{
	Columns(1);

	Separator();
}
