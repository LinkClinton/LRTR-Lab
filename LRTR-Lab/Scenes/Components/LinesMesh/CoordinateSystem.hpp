#pragma once

#include "LinesMesh.hpp"

namespace LRTR {

	enum class Axis : UInt32 {
		eX = 0,
		eY = 1,
		eZ = 2
	};
	
	class CoordinateSystem : public LinesMesh {
	public:
		CoordinateSystem();

		explicit CoordinateSystem(
			const std::array<Vector3f, 3>& axes,
			const std::array<ColorF, 3>& colors,
			const float length = 1.0f);

		~CoordinateSystem() = default;
		
		auto axis(const Axis& axis) const -> Vector3f;

		auto color(const Axis& axis) const -> ColorF;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	};
	
}