#pragma once

#include "LightSource.hpp"

namespace LRTR {

	class PointLightSource : public LightSource {
	public:
		explicit PointLightSource(const Vector3f& intensity);

		auto intensity() const noexcept -> Vector3f;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	};
	
}