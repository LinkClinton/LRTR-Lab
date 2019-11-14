#pragma once

#include "../Components/Transform.hpp"

namespace LRTR {

	class Camera {
	public:
		Camera() = default;
	};

	class ProjectiveCamera : public Camera {
	public:
		ProjectiveCamera() = default;
	};

	class PerspectiveCamera : public ProjectiveCamera {
	public:
		PerspectiveCamera() = default;
	protected:
	};
}