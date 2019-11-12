#pragma once

#include "UIComponent.hpp"

#include <CodeRed/Interface/GpuResource/GpuTexture.hpp>

namespace LRTR {

	class SceneViewUIComponent : public UIComponent {
	public:
		explicit SceneViewUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~SceneViewUIComponent() = default;

		auto sceneTexture() const noexcept -> std::shared_ptr<CodeRed::GpuTexture>;
	private:
		void update();
	private:
		std::shared_ptr<CodeRed::GpuTexture> mSceneTexture;
	};
	
}