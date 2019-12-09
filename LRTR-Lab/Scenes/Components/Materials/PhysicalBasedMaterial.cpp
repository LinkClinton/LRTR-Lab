#include "PhysicalBasedMaterial.hpp"

#include "../../../Shared/Textures/ConstantTexture.hpp"
#include "../../../Shared/Math/Math.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::PhysicalBasedMaterial::PhysicalBasedMaterial(
	const std::shared_ptr<Texture>& metallic,
	const std::shared_ptr<Texture>& baseColor, 
	const std::shared_ptr<Texture>& roughness,
	const std::shared_ptr<Texture>& occlusion, 
	const std::shared_ptr<Texture>& normalMap) :
	mMetallic(metallic), mBaseColor(baseColor), mRoughness(roughness),
	mOcclusion(occlusion), mNormalMap(normalMap)
{
}

auto LRTR::PhysicalBasedMaterial::metallic() const noexcept -> std::shared_ptr<Texture>
{
	return mMetallic;
}

auto LRTR::PhysicalBasedMaterial::baseColor() const noexcept -> std::shared_ptr<Texture>
{
	return mBaseColor;
}

auto LRTR::PhysicalBasedMaterial::roughness() const noexcept -> std::shared_ptr<Texture>
{
	return mRoughness;
}

auto LRTR::PhysicalBasedMaterial::occlusion() const noexcept -> std::shared_ptr<Texture>
{
	return mOcclusion;
}

auto LRTR::PhysicalBasedMaterial::normalMap() const noexcept -> std::shared_ptr<Texture>
{
	return mNormalMap;
}

auto LRTR::PhysicalBasedMaterial::typeName() const noexcept -> std::string
{
	return "PhysicalBasedMaterial";
}

auto LRTR::PhysicalBasedMaterial::typeIndex() const noexcept -> std::type_index
{
	return typeid(PhysicalBasedMaterial);
}

void LRTR::PhysicalBasedMaterial::onProperty()
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
