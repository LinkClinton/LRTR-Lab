#pragma once

#include "Components/MotionProperty.hpp"
#include "Camera.hpp"

namespace LRTR {

	class MotionCamera : public PerspectiveCamera {
	public:
		explicit MotionCamera(
			const std::shared_ptr<TransformWrap>& transform,
			const std::shared_ptr<Perspective>& perspective,
			const std::shared_ptr<MotionProperty>& motion);

		~MotionCamera() = default;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	};
	
}
