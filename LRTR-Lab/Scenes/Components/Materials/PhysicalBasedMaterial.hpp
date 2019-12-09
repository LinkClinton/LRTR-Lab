#pragma once

#include "../../../Shared/Textures/Texture.hpp"

#include "Material.hpp"

namespace LRTR {

	class PhysicalBasedMaterial : public Material {
	public:
		explicit PhysicalBasedMaterial() = default;

		explicit PhysicalBasedMaterial(
			const std::shared_ptr<Texture>& metallic,
			const std::shared_ptr<Texture>& baseColor,
			const std::shared_ptr<Texture>& roughness,
			const std::shared_ptr<Texture>& occlusion = nullptr,
			const std::shared_ptr<Texture>& normalMap = nullptr);

		auto metallic() const noexcept -> std::shared_ptr<Texture>;

		auto baseColor() const noexcept -> std::shared_ptr<Texture>;

		auto roughness() const noexcept -> std::shared_ptr<Texture>;

		auto occlusion() const noexcept -> std::shared_ptr<Texture>;

		auto normalMap() const noexcept -> std::shared_ptr<Texture>;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		std::shared_ptr<Texture> mMetallic;
		std::shared_ptr<Texture> mBaseColor;
		std::shared_ptr<Texture> mRoughness;
		std::shared_ptr<Texture> mOcclusion;
		std::shared_ptr<Texture> mNormalMap;
	};
	
}