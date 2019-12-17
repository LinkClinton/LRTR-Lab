#pragma once

#include "../../../Shared/Textures/ConstantTexture.hpp"
#include "../../../Shared/Textures/ImageTexture.hpp"

#include "Material.hpp"

namespace LRTR {

	class PhysicalBasedMaterial : public Material {
	public:
		explicit PhysicalBasedMaterial();

		explicit PhysicalBasedMaterial(
			const std::shared_ptr<ImageTexture>& metallic,
			const std::shared_ptr<ImageTexture>& baseColor,
			const std::shared_ptr<ImageTexture>& roughness,
			const std::shared_ptr<ImageTexture>& occlusion = nullptr,
			const std::shared_ptr<ImageTexture>& normalMap = nullptr);

		explicit PhysicalBasedMaterial(
			const std::shared_ptr<ConstantTexture4F>& metallic,
			const std::shared_ptr<ConstantTexture4F>& baseColor,
			const std::shared_ptr<ConstantTexture4F>& roughness);

		explicit PhysicalBasedMaterial(
			const std::shared_ptr<ConstantTexture4F>& metallicFactor,
			const std::shared_ptr<ConstantTexture4F>& baseColorFactor,
			const std::shared_ptr<ConstantTexture4F>& roughnessFactor,
			const std::shared_ptr<ImageTexture>& metallicTexture,
			const std::shared_ptr<ImageTexture>& baseColorTexture,
			const std::shared_ptr<ImageTexture>& roughnessTexture,
			const std::shared_ptr<ImageTexture>& occlusionTexture = nullptr,
			const std::shared_ptr<ImageTexture>& normalMapTexture = nullptr);

		auto metallicFactor() const noexcept -> std::shared_ptr<ConstantTexture4F>;

		auto baseColorFactor() const noexcept -> std::shared_ptr<ConstantTexture4F>;

		auto roughnessFactor() const noexcept -> std::shared_ptr<ConstantTexture4F>;

		auto metallicTexture() const noexcept -> std::shared_ptr<ImageTexture>;

		auto baseColorTexture() const noexcept -> std::shared_ptr<ImageTexture>;

		auto roughnessTexture() const noexcept -> std::shared_ptr<ImageTexture>;

		auto occlusionTexture() const noexcept -> std::shared_ptr<ImageTexture>;

		auto normalMapTexture() const noexcept -> std::shared_ptr<ImageTexture>;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		std::shared_ptr<ImageTexture> mMetallicTexture;
		std::shared_ptr<ImageTexture> mBaseColorTexture;
		std::shared_ptr<ImageTexture> mRoughnessTexture;
		std::shared_ptr<ImageTexture> mOcclusionTexture;
		std::shared_ptr<ImageTexture> mNormalMapTexture;

		std::shared_ptr<ConstantTexture4F> mMetallicFactor;
		std::shared_ptr<ConstantTexture4F> mBaseColorFactor;
		std::shared_ptr<ConstantTexture4F> mRoughnessFactor;
	};
	
}