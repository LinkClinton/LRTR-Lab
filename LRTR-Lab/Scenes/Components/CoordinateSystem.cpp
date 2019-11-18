#include "CoordinateSystem.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

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
	const float length) : mAxes(axes), mColors(colors), mLength(length)
{
}

auto LRTR::CoordinateSystem::axes() const noexcept -> const std::array<Vector3f, 3>& 
{
	return mAxes;
}

auto LRTR::CoordinateSystem::colors() const noexcept -> const std::array<ColorF, 3>& 
{
	return mColors;
}

auto LRTR::CoordinateSystem::axis(const Axis& axis) const -> Vector3f
{
	return mAxes[static_cast<UInt32>(axis)];
}

auto LRTR::CoordinateSystem::color(const Axis& axis) const -> ColorF
{
	return mColors[static_cast<UInt32>(axis)];
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
	const static auto GenColumn = [](const char* text, const char* id, float* data, const char* format = "%.3f")
	{
		static std::string head = "##";

		ImGui::AlignTextToFramePadding();

		ImGui::Text(text); ImGui::NextColumn();
		ImGui::InputFloat((head + id + text).c_str(), data, 0, 0, format); ImGui::NextColumn();
	};

	const static auto EditFlags =
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_Float;

	const char* AxesName[] = {
		"X",
		"Y",
		"Z"
	};

	static auto currentAxis = static_cast<size_t>(Axis::eX);

	ImGui::Columns(2, "Combo");
	ImGui::Separator();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Axis");
	ImGui::NextColumn();

	if (ImGui::BeginCombo("##Axis", AxesName[currentAxis])) {
		for (size_t index = 0; index < mAxes.size(); index++) {
			const auto selected = (currentAxis == index);
			
			if (ImGui::Selectable(AxesName[index], selected))
				currentAxis = index;
			if (selected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	
	ImGui::NextColumn();

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::Columns(2, AxesName[currentAxis]);
	ImGui::Separator();

	GenColumn("Location X", "0", &mAxes[currentAxis].x);
	GenColumn("         Y", "0", &mAxes[currentAxis].y);
	GenColumn("         Z", "0", &mAxes[currentAxis].z);

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Color    RGBA");
	ImGui::NextColumn();
	ImGui::ColorEdit4("ColorEdit4",reinterpret_cast<float*>(&mColors[currentAxis]), EditFlags);
	ImGui::NextColumn();

	ImGui::Columns(2, "Length");
	ImGui::Separator();
	GenColumn("Length", "0", &mLength);

	ImGui::Columns(2, "Visibility");
	ImGui::Separator();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Visibility");
	ImGui::NextColumn();
	ImGui::Checkbox("##Visibility", &mVisibility);
	ImGui::NextColumn();
	
	ImGui::Columns(1);
	ImGui::Separator();

	ImGui::PopStyleColor();
}
