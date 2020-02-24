#include "MotionCamera.hpp"

LRTR::MotionCamera::MotionCamera(
	const std::shared_ptr<TransformWrap>& transform,
	const std::shared_ptr<Perspective>& perspective, 
	const std::shared_ptr<MotionProperty>& motion) : PerspectiveCamera(transform, perspective)
{
	addComponent(motion);
}

auto LRTR::MotionCamera::typeName() const noexcept -> std::string
{
	return "MotionCamera";
}

auto LRTR::MotionCamera::typeIndex() const noexcept -> std::type_index
{
	return typeid(MotionCamera);
}
