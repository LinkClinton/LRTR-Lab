#include "CoordinateSystem.hpp"

#include "../../Extensions/ImGui/ImGui.hpp"

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

	ImGui::BeginPropertyTable("Combo");
	ImGui::Property("Axis", [&]()
		{
			if (ImGui::BeginCombo("##Axis", AxesName[currentAxis])) {
				for (size_t index = 0; index < mAxes.size(); index++) {
					const auto selected = (currentAxis == index);

					if (ImGui::Selectable(AxesName[index], selected))
						currentAxis = index;
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		});
	
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));

	ImGui::BeginPropertyTable(AxesName[currentAxis]);
	ImGui::Property("Location X", [&]() { ImGui::InputFloat("##X", &mAxes[currentAxis].x); });
	ImGui::Property("         Y", [&]() { ImGui::InputFloat("##Y", &mAxes[currentAxis].y); });
	ImGui::Property("         Z", [&]() { ImGui::InputFloat("##Z", &mAxes[currentAxis].z); });
	ImGui::Property("Color    RGBA", [&]()
		{
			ImGui::ColorEdit4("##ColorEdit4", reinterpret_cast<float*>(&mColors[currentAxis]), EditFlags);
		});

	ImGui::BeginPropertyTable("Length");
	ImGui::Property("Length", [&]() { ImGui::InputFloat("##Length", &mLength); });

	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() {ImGui::Checkbox("##Visibility", &mVisibility); });
	
	ImGui::PopStyleColor();
	
	ImGui::EndPropertyTable();

	mAxes[currentAxis] = MathUtility::normalize(mAxes[currentAxis]);
}
