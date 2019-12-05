#include "PhysicalBaseMaterial.hpp"

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
	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() {ImGui::Checkbox("##Visibility", &mVisibility); });

	ImGui::EndPropertyTable();
}
