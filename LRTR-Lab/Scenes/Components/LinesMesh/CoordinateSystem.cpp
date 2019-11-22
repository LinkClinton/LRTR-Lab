#include "CoordinateSystem.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::CoordinateSystem::CoordinateSystem() :
	CoordinateSystem(
		{
			Vector3f(1,0,0),
			Vector3f(0,1,0),
			Vector3f(0,0,1)
		},
		{
			ColorF(1, 0, 0, 1),
			ColorF(0, 1, 0, 1),
			ColorF(0, 0, 1, 1)
		})
{

}

LRTR::CoordinateSystem::CoordinateSystem(
	const std::array<Vector3f, 3>& axes, 
	const std::array<ColorF, 3>& colors,
	const float length)
{
	for (size_t index = 0; index < 3; index++)
		mLines.push_back(Line(Vector3f(0), axes[index], colors[index]));
}

auto LRTR::CoordinateSystem::axis(const Axis& axis) const -> Vector3f
{
	return mLines[static_cast<size_t>(axis)].End;
}

auto LRTR::CoordinateSystem::color(const Axis& axis) const -> ColorF
{
	return mLines[static_cast<size_t>(axis)].Color;
}

auto LRTR::CoordinateSystem::typeName() const noexcept -> std::string
{
	return "CoordinateSystem";
}

auto LRTR::CoordinateSystem::typeIndex() const noexcept -> std::type_index
{
	return typeid(CoordinateSystem);
}

void LRTR::CoordinateSystem::onProperty()
{
	static const auto EditFlags =
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_Float;

	static const char* AxesName[] = {
		"X",
		"Y",
		"Z"
	};

	static auto currentAxis = static_cast<size_t>(Axis::eX);

	ImGui::AxisProperty(3, currentAxis);
	
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));

	ImGui::BeginPropertyTable(AxesName[currentAxis]);
	ImGui::Property("Location X", [&]() { ImGui::InputFloat("##X", &mLines[currentAxis].End.x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y", &mLines[currentAxis].End.y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z", &mLines[currentAxis].End.z); });
	ImGui::Property("Color    RGBA", [&]()
		{
			ImGui::ColorEdit4("##ColorEdit4", reinterpret_cast<float*>(&mLines[currentAxis].Color), EditFlags);
		});

	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() { ImGui::Checkbox("##Visibility", &mVisibility); });
	
	ImGui::PopStyleColor();
	
	ImGui::EndPropertyTable();
}
