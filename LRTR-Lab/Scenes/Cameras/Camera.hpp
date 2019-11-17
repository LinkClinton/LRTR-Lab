#pragma once

#include "../Components/TransformWrap.hpp"
#include "../Shape.hpp"

#include "Components/Perspective.hpp"

namespace LRTR {

	class Camera : public Shape {
	public:
		Camera();

		explicit Camera(const std::shared_ptr<TransformWrap>& transform);

		~Camera() = default;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	};

	class ProjectiveCamera : public Camera {
	public:
		explicit ProjectiveCamera(
			const std::shared_ptr<TransformWrap>& transform,
			const std::shared_ptr<Projective>& projective);
		
		~ProjectiveCamera() = default;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	};

	class PerspectiveCamera : public ProjectiveCamera {
	public:
		PerspectiveCamera();

		explicit PerspectiveCamera(
			const std::shared_ptr<TransformWrap>& transform,
			const std::shared_ptr<Perspective>& perspective);

		~PerspectiveCamera() = default;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	};
}