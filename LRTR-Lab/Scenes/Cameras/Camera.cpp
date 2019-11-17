#include "Camera.hpp"

LRTR::Camera::Camera() : Camera(std::make_shared<TransformWrap>())
{
}

LRTR::Camera::Camera(const std::shared_ptr<TransformWrap>& transform)
{
	addComponent(transform);
}

auto LRTR::Camera::typeName() const noexcept -> std::string
{
	return "Camera";
}

auto LRTR::Camera::typeIndex() const noexcept -> std::type_index
{
	return typeid(Camera);
}

LRTR::ProjectiveCamera::ProjectiveCamera(
	const std::shared_ptr<TransformWrap>& transform,
	const std::shared_ptr<Projective>& projective) :
	Camera(transform)
{
	addComponent(projective);
}

auto LRTR::ProjectiveCamera::typeName() const noexcept -> std::string
{
	return "Camera";
}

auto LRTR::ProjectiveCamera::typeIndex() const noexcept -> std::type_index
{
	return typeid(ProjectiveCamera);
}

LRTR::PerspectiveCamera::PerspectiveCamera() :
	PerspectiveCamera(
		std::make_shared<TransformWrap>(),
		std::make_shared<Perspective>())
{
}

LRTR::PerspectiveCamera::PerspectiveCamera(
	const std::shared_ptr<TransformWrap>& transform,
	const std::shared_ptr<Perspective>& perspective) :
	ProjectiveCamera(transform, perspective)
{
}

auto LRTR::PerspectiveCamera::typeName() const noexcept -> std::string
{
	return "Camera";
}

auto LRTR::PerspectiveCamera::typeIndex() const noexcept -> std::type_index
{
	return typeid(PerspectiveCamera);
}

