#pragma once

#include "../../../../Shared/Accelerators/Group.hpp"
#include "UIComponent.hpp"

namespace LRTR {

	class MainMenuUIComponent : public UIComponent {
	public:
		explicit MainMenuUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~MainMenuUIComponent() = default;
	private:
		void update();
	private:
		using Menu = StringGroup<std::string>;

		StringGroup<Menu> mWindowMenus;
	};

}