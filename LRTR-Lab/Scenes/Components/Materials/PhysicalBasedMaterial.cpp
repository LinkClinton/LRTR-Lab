#include "PhysicalBasedMaterial.hpp"

#include "../../../Shared/Textures/ConstantTexture.hpp"
#include "../../../Shared/Math/Math.hpp"

#include "../../../Extensions/ImGui/ImGui.hpp"

LRTR::PhysicalBasedMaterial::PhysicalBasedMaterial() :
	PhysicalBasedMaterial(
		std::make_shared<ConstantTexture4F>(Vector4f(1)),
		std::make_shared<ConstantTexture4F>(Vector4f(1)),
		std::make_shared<ConstantTexture4F>(Vector4f(1)))
{
}

LRTR::PhysicalBasedMaterial::PhysicalBasedMaterial(
	const std::shared_ptr<ImageTexture>& metallic,
	const std::shared_ptr<ImageTexture>& baseColor, 
	const std::shared_ptr<ImageTexture>& roughness,
	const std::shared_ptr<ImageTexture>& occlusion, 
	const std::shared_ptr<ImageTexture>& normalMap) :
	PhysicalBasedMaterial(nullptr, nullptr, nullptr, 
		metallic, baseColor, roughness, occlusion, normalMap)
{
}

LRTR::PhysicalBasedMaterial::PhysicalBasedMaterial(
	const std::shared_ptr<ConstantTexture4F>& metallic,
	const std::shared_ptr<ConstantTexture4F>& baseColor, 
	const std::shared_ptr<ConstantTexture4F>& roughness) :
	mMetallicFactor(metallic), mBaseColorFactor(baseColor), mRoughnessFactor(roughness)
{
	
}

LRTR::PhysicalBasedMaterial::PhysicalBasedMaterial(
	const std::shared_ptr<ConstantTexture4F>& metallicFactor,
	const std::shared_ptr<ConstantTexture4F>& baseColorFactor,
	const std::shared_ptr<ConstantTexture4F>& roughnessFactor,
	const std::shared_ptr<ImageTexture>& metallicTexture,
	const std::shared_ptr<ImageTexture>& baseColorTexture,
	const std::shared_ptr<ImageTexture>& roughnessTexture,
	const std::shared_ptr<ImageTexture>& occlusionTexture,
	const std::shared_ptr<ImageTexture>& normalMapTexture) :
	mMetallicTexture(metallicTexture), mBaseColorTexture(baseColorTexture), mRoughnessTexture(roughnessTexture),
	mOcclusionTexture(occlusionTexture), mNormalMapTexture(normalMapTexture), mMetallicFactor(metallicFactor),
	mBaseColorFactor(baseColorFactor), mRoughnessFactor(roughnessFactor)
{
	if (mMetallicFactor == nullptr) mMetallicFactor = std::make_shared<ConstantTexture4F>(Vector4f(1));
	if (mBaseColorFactor == nullptr) mBaseColorFactor = std::make_shared<ConstantTexture4F>(Vector4f(1));
	if (mRoughnessFactor == nullptr) mRoughnessFactor = std::make_shared<ConstantTexture4F>(Vector4f(1));
}

auto LRTR::PhysicalBasedMaterial::metallicFactor() const noexcept -> std::shared_ptr<ConstantTexture4F>
{
	return mMetallicFactor;
}

auto LRTR::PhysicalBasedMaterial::baseColorFactor() const noexcept -> std::shared_ptr<ConstantTexture4F>
{
	return mBaseColorFactor;
}

auto LRTR::PhysicalBasedMaterial::roughnessFactor() const noexcept -> std::shared_ptr<ConstantTexture4F>
{
	return mRoughnessFactor;
}

auto LRTR::PhysicalBasedMaterial::metallicTexture() const noexcept -> std::shared_ptr<ImageTexture>
{
	return mMetallicTexture;
}

auto LRTR::PhysicalBasedMaterial::baseColorTexture() const noexcept -> std::shared_ptr<ImageTexture>
{
	return mBaseColorTexture;
}

auto LRTR::PhysicalBasedMaterial::roughnessTexture() const noexcept -> std::shared_ptr<ImageTexture>
{
	return mRoughnessTexture;
}

auto LRTR::PhysicalBasedMaterial::occlusionTexture() const noexcept -> std::shared_ptr<ImageTexture>
{
	return mOcclusionTexture;
}

auto LRTR::PhysicalBasedMaterial::normalMapTexture() const noexcept -> std::shared_ptr<ImageTexture>
{
	return mNormalMapTexture;
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

	ImGui::BeginPropertyTable("Visibility");
	ImGui::Property("Visibility", [&]() {ImGui::Checkbox("##Visibility", &mVisibility); });

	ImGui::EndPropertyTable();

	ImGui::PopStyleColor();
}
