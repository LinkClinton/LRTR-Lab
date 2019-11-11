#pragma once

#include "UIComponent.hpp"

namespace LRTR {

	class MainMenuUIComponent : public UIComponent {
	public:
		explicit MainMenuUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~MainMenuUIComponent() = default;
	private:
		void update();
	};

}