#pragma once

#include "UIComponent.hpp"

namespace LRTR {

	class SceneShapeUIComponent : public UIComponent {
	public:
		explicit SceneShapeUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~SceneShapeUIComponent() = default;
	private:
		void update();
	};
	
}