#pragma once

#include "../../../Shared/Rectangle.hpp"

#include "LinesMesh.hpp"

namespace LRTR {

	class LinesGrid : public LinesMesh {
	public:
		LinesGrid() = default;

		explicit LinesGrid(
			const RectangleF& area,
			const size_t xCount,
			const size_t yCount,
			const ColorF& color = ColorF(1, 1, 1, 1));

		explicit LinesGrid(
			const RectangleF& area,
			const size_t xCount,
			const size_t yCount,
			const Vector3f& xAxis,
			const Vector3f& yAxis,
			const ColorF& color = ColorF(1, 1, 1, 1));
		
		explicit LinesGrid(
			const RectangleF& area,
			const size_t xCount,
			const size_t yCount,
			const Vector3f& xAxis,
			const Vector3f& yAxis,
			const Vector3f& origin,
			const ColorF& color = ColorF(1, 1, 1, 1));

		~LinesGrid() = default;
		
		auto origin() const noexcept -> Vector3f;

		auto xAxis() const noexcept -> Vector3f;

		auto yAxis() const noexcept -> Vector3f;

		auto area() const noexcept -> RectangleF;
		
		auto xCount() const noexcept -> size_t;

		auto yCount() const noexcept -> size_t;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;

		void updateLines();
	private:
		Vector3f mOrigin = Vector3f(0);
		Vector3f mXAxis = Vector3f(1, 0, 0);
		Vector3f mYAxis = Vector3f(0, 1, 0);
		ColorF mColor = ColorF(1, 1, 1, 1);

		RectangleF mArea;

		size_t mXCount = 1;
		size_t mYCount = 1;
	};
	
}