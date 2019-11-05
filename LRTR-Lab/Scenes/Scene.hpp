#pragma once

#include "../Core/Noncopyable.hpp"
#include "Cameras/Camera.hpp"

#include <unordered_map>
#include <memory>

namespace LRTR {

	class Scene : public Noncopyable {
	public:
	private:
		virtual void update(float delta);

		virtual void render(float delta);
	private:
		using SceneCamera = ProjectiveCamera;
		
		std::shared_ptr<SceneCamera> mMainCamera;
	};
	
}