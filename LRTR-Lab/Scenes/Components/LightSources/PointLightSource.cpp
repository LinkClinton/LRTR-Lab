#include "PointLightSource.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::PointLightSource::PointLightSource(const Vector3f& intensity) :
	LightSource(intensity)
{
}

auto LRTR::PointLightSource::intensity() const noexcept -> Vector3f
{
	return mIntensity;
}

auto LRTR::PointLightSource::typeName() const noexcept -> std::string
{
	return "PointLight";
}

auto LRTR::PointLightSource::typeIndex() const noexcept -> std::type_index
{
	return typeid(PointLightSource);
}

void LRTR::PointLightSource::onProperty()
{
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable("Intensity");

	ImGui::Property("Intensity X", [&]() { ImGui::InputFloat("##X", &mIntensity.x); });
	ImGui::Property("          Y", [&]() { ImGui::InputFloat("##Y", &mIntensity.y); });
	ImGui::Property("          Z", [&]() { ImGui::InputFloat("##Z", &mIntensity.z); });

	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();
}
