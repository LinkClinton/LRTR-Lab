#pragma once

#include "../Core/Noncopyable.hpp"

#include "Cameras/Camera.hpp"
#include "Shape.hpp"

#include <CodeRed/Interface/GpuGraphicsCommandList.hpp>

namespace LRTR {

	using SceneCamera = ProjectiveCamera;
	
	class System : public Noncopyable {
	public:
		System() = default;

		~System() = default;
	};

	class UpdateSystem : public System {
	public:
		UpdateSystem() = default;

		~UpdateSystem() = default;

		virtual void update(
			const StringGroup<std::shared_ptr<Shape>>& shapes,
			const float delta) = 0;
	};

	class RenderSystem : public System {
	public:
		RenderSystem() = default;

		~RenderSystem() = default;

		virtual void render(
			const std::shared_ptr<CodeRed::GpuGraphicsCommandList>& commandList,
			const std::shared_ptr<SceneCamera>& camera,
			const StringGroup<std::shared_ptr<Shape>>& shapes,
			const float delta) = 0;
	};
	
}