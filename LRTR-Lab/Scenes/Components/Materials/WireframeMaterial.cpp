#include "WireframeMaterial.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::WireframeMaterial::WireframeMaterial(const ColorF& color) :
	mColor(color)
{
	
}

auto LRTR::WireframeMaterial::color() const noexcept -> ColorF
{
	return mColor;
}

auto LRTR::WireframeMaterial::typeName() const noexcept -> std::string
{
	return "WireframeMaterial";
}

auto LRTR::WireframeMaterial::typeIndex() const noexcept -> std::type_index
{
	return typeid(WireframeMaterial);
}

void LRTR::WireframeMaterial::onProperty()
{
	const static auto EditFlags =
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_Float;

	ImGui::BeginPropertyTable("Color");
	ImGui::Property("Color    RGBA", [&]()
		{
			ImGui::ColorEdit4("##ColorEdit4", reinterpret_cast<float*>(&mColor), EditFlags);
		});

	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() {ImGui::Checkbox("##Visibility", &IsRendered); });
	
	ImGui::EndPropertyTable();
}
