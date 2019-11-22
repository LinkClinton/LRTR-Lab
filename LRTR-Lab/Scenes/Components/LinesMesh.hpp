#pragma once

#include "../../Core/Renderable.hpp"

#include "../../Shared/Math/Math.hpp"
#include "../../Shared/Color.hpp"
#include "../Component.hpp"

namespace LRTR {

	struct Line {
		Vector3f Begin = Vector3f(0);
		Vector3f End = Vector3f(1);
		ColorF Color = ColorF(1, 1, 1, 1);

		Line() = default;

		Line(
			const Vector3f& begin,
			const Vector3f& end,
			const ColorF& color);
	};
	
	class LinesMesh : public Component, public Renderable {
	public:
		LinesMesh() = default;
	
		explicit LinesMesh(const std::vector<Line>& lines);

		~LinesMesh() = default;

		auto line(size_t index) const -> Line;
		
		auto liens() const noexcept -> const std::vector<Line>&;

		auto size() const -> size_t;
		
		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		void onProperty() override;
	protected:
		std::vector<Line> mLines;

		size_t mCurrentLine = 0;
	};
	
}