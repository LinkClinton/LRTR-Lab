#include "PhysicalBaseMaterial.hpp"

#include "../../../Shared/Textures/ConstantTexture.hpp"
#include "../../../Shared/Math/Math.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::PhysicalBaseMaterial::PhysicalBaseMaterial(
	const std::shared_ptr<Texture>& metallic,
	const std::shared_ptr<Texture>& baseColor, 
	const std::shared_ptr<Texture>& roughness,
	const std::shared_ptr<Texture>& occlusion, 
	const std::shared_ptr<Texture>& normalMap) :
	mMetallic(metallic), mBaseColor(baseColor), mRoughness(roughness),
	mOcclusion(occlusion), mNormalMap(normalMap)
{
}

auto LRTR::PhysicalBaseMaterial::metallic() const noexcept -> std::shared_ptr<Texture>
{
	return mMetallic;
}

auto LRTR::PhysicalBaseMaterial::baseColor() const noexcept -> std::shared_ptr<Texture>
{
	return mBaseColor;
}

auto LRTR::PhysicalBaseMaterial::roughness() const noexcept -> std::shared_ptr<Texture>
{
	return mRoughness;
}

auto LRTR::PhysicalBaseMaterial::occlusion() const noexcept -> std::shared_ptr<Texture>
{
	return mOcclusion;
}

auto LRTR::PhysicalBaseMaterial::normalMap() const noexcept -> std::shared_ptr<Texture>
{
	return mNormalMap;
}

auto LRTR::PhysicalBaseMaterial::typeName() const noexcept -> std::string
{
	return "PhysicalBaseMaterial";
}

auto LRTR::PhysicalBaseMaterial::typeIndex() const noexcept -> std::type_index
{
	return typeid(PhysicalBaseMaterial);
}

void LRTR::PhysicalBaseMaterial::onProperty()
{
	const static auto EditFlags =
		ImGuiColorEditFlags_NoInputs |
		ImGuiColorEditFlags_NoLabel |
		ImGuiColorEditFlags_AlphaPreview |
		ImGuiColorEditFlags_Float;

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.1f));
	
	ImGui::BeginPropertyTable("BaseColor");

	ImGui::Property("BaseColor", [&]()
		{
			//for non-texture material property
			if (std::dynamic_pointer_cast<ConstantTexture<Vector4f>>(mBaseColor)) {
				auto baseColor = std::static_pointer_cast<ConstantTexture<Vector4f>>(mBaseColor)->value();
				
				ImGui::ColorEdit4("##BaseColor", reinterpret_cast<float*>(&baseColor), EditFlags);
			}else {
				//todo
			}
		});

	ImGui::BeginPropertyTable("Roughness");

	ImGui::Property("Roughness", [&]()
		{
			//for non-texture material property
			if (std::dynamic_pointer_cast<ConstantTexture<Vector1f>>(mRoughness)) {
				auto roughness = std::static_pointer_cast<ConstantTexture<Vector1f>>(mRoughness)->value();

				ImGui::InputFloat("##Roughness", &roughness.x, 0, 0, 3, ImGuiInputTextFlags_ReadOnly);
			}
			else {
				//todo
			}
		});

	ImGui::BeginPropertyTable("Metallic");

	ImGui::Property("Metallic", [&]()
		{
			//for non-texture material property
			if (std::dynamic_pointer_cast<ConstantTexture<Vector1f>>(mMetallic)) {
				auto metallic = std::static_pointer_cast<ConstantTexture<Vector1f>>(mMetallic)->value();

				ImGui::InputFloat("##Metallic", &metallic.x, 0, 0, 3, ImGuiInputTextFlags_ReadOnly);
			}
			else {
				//todo
			}
		});

	//normal map and occlusion are textures, so we will support next time.
	
	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() {ImGui::Checkbox("##Visibility", &mVisibility); });

	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();
}
