#pragma once

#include "../../../Shared/Color.hpp"

#include "Material.hpp"

namespace LRTR {

	class WireframeMaterial : public Material {
	public:
		WireframeMaterial() = default;

		explicit WireframeMaterial(const ColorF& color);

		auto color() const noexcept -> ColorF;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		ColorF mColor = ColorF(1, 1, 1, 1);
	};
	
}