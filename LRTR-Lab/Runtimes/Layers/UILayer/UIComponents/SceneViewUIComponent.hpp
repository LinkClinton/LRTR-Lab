#pragma once

#include "UIComponent.hpp"

namespace LRTR {

	class SceneViewUIComponent : public UIComponent {
	public:
		explicit SceneViewUIComponent(const std::shared_ptr<UILayerSharing>& sharing);

		~SceneViewUIComponent() = default;
	private:
		void update();
	};
	
}