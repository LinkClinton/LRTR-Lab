#include "SkyBox.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::SkyBox::SkyBox(const std::shared_ptr<CodeRed::GpuTexture>& cubeMap) :
	mCubeMap(cubeMap)
{

}

auto LRTR::SkyBox::cubeMap() const noexcept -> std::shared_ptr<CodeRed::GpuTexture>
{
	return mCubeMap;
}

auto LRTR::SkyBox::typeName() const noexcept -> std::string
{
	return "SkyBox";
}

auto LRTR::SkyBox::typeIndex() const noexcept -> std::type_index
{
	return typeid(SkyBox);
}

void LRTR::SkyBox::onProperty()
{
	const static auto EditFlags =
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_Float;

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));

	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() {ImGui::Checkbox("##Visibility", &mVisibility); });

	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();
}
