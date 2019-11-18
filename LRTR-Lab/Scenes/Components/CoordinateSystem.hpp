#pragma once

#include "../../Core/Renderable.hpp"

#include "../../Shared/Math/Math.hpp"
#include "../../Shared/Color.hpp"
#include "../Component.hpp"

namespace LRTR {

	enum class Axis : UInt32 {
		eX = 0,
		eY = 1,
		eZ = 2
	};
	
	class CoordinateSystem : public Component, public Renderable {
	public:
		CoordinateSystem();

		explicit CoordinateSystem(
			const std::array<Vector3f, 3>& axes,
			const std::array<ColorF, 3>& colors,
			const float length = 1.0f);
		
		auto axes() const noexcept -> const std::array<Vector3f, 3>&;

		auto colors() const noexcept -> const std::array<ColorF, 3>&;

		auto axis(const Axis& axis) const -> Vector3f;

		auto color(const Axis& axis) const -> ColorF;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	private:
		std::array<Vector3f, 3> mAxes;
		std::array<ColorF, 3> mColors;

		float mLength;
	};
	
}