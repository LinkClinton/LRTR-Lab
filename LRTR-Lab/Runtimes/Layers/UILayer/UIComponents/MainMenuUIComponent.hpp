#pragma once

#include "UIComponent.hpp"

namespace LRTR {

	class MainMenuUIComponent : public UIComponent {
	public:
		MainMenuUIComponent();

		~MainMenuUIComponent() = default;
	private:
		void updateUI();
	};

}