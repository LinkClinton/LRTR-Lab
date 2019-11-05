#pragma once

#include "../Transform.hpp"

namespace LRTR {

	class Camera {
	public:
		Camera() = default;

		explicit Camera(const Transform& cameraToWorld) :
			mCameraToWorld(cameraToWorld) {}

		virtual ~Camera() = default;
		
		auto toWorld() const noexcept -> Transform { return mCameraToWorld; }
	protected:
		Transform mCameraToWorld;
	};

	class ProjectiveCamera : public Camera {
	public:
		ProjectiveCamera() = default;

		explicit ProjectiveCamera(
			const Transform& cameraToWorld,
			const Transform& cameraToScreen) :
			Camera(cameraToWorld),
			mCameraToScreen(cameraToScreen) {}

		auto toScreen() const noexcept -> Transform { return mCameraToScreen; }
	protected:
		Transform mCameraToScreen;
	};

	class PerspectiveCamera : public ProjectiveCamera {
	public:
		PerspectiveCamera() = default;

		explicit PerspectiveCamera(
			const Transform& cameraToWorld,
			const Real fovy,
			const size_t width,
			const size_t height) :
			ProjectiveCamera(cameraToWorld, 
				Transform::perspectiveFov(fovy, 
					static_cast<Real>(width), 
					static_cast<Real>(height), 0.f, 1000.0f)) {}
	protected:
	};
}