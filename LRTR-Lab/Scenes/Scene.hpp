#pragma once

#include "../Shared/Accelerators/Group.hpp"
#include "../Core/Noncopyable.hpp"
#include "Cameras/Camera.hpp"

#include <unordered_map>
#include <memory>

namespace LRTR {

	using SceneCamera = ProjectiveCamera;

	class Scene : public Noncopyable {
	public:
		Scene() = default;

		virtual ~Scene() = default;

		void setMainCamera(const std::string& name);

		auto mainCamera() const noexcept -> std::shared_ptr<SceneCamera>;
		
		auto cameras() noexcept -> StringGroup<std::shared_ptr<SceneCamera>>&;
	private:
		virtual void update(float delta);

		virtual void render(float delta);
	private:
		std::shared_ptr<SceneCamera> mMainCamera;

		StringGroup<std::shared_ptr<SceneCamera>> mCameras;
	};
	
}