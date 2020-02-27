#include "MotionProperty.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::MotionProperty::MotionProperty(const float sensitivity, const float speed, 
	const std::array<bool, 3>& axes) :
	mSensitivity(sensitivity), mSpeed(speed), mAxes(axes)
{
}

auto LRTR::MotionProperty::typeName() const noexcept -> std::string
{
	return "MotionProperty";
}

auto LRTR::MotionProperty::typeIndex() const noexcept -> std::type_index
{
	return typeid(MotionProperty);
}

void LRTR::MotionProperty::onProperty()
{
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));

	ImGui::BeginPropertyTable("Sensitivity");
	ImGui::Property("Sensitivity", [&]() { ImGui::InputFloat("##Sensitivity", &mSensitivity); });
	ImGui::BeginPropertyTable("Speed");
	ImGui::Property("Speed", [&]() {ImGui::InputFloat("##Speed", &mSpeed); });
	ImGui::BeginPropertyTable("Axes");
	ImGui::Property("Axis X", [&]() { ImGui::Checkbox("##Axis X", &mAxes[0]); });
	ImGui::Property("Axis Y", [&]() { ImGui::Checkbox("##Axis Y", &mAxes[1]); });
	ImGui::Property("Axis Z", [&]() { ImGui::Checkbox("##Axis Z", &mAxes[2]); });
	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();
}
