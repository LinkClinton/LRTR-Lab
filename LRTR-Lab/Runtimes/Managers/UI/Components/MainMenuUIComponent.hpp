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

		auto nameIndex(const std::string& name) const -> size_t;

		void initializeWindowsMenus();
	private:
		using Component = std::pair<std::string, std::string>;
		using Components = std::vector<Component>;
		using Menu = std::pair<std::string, Components>;

		StringGroup<size_t> mNameIndices;
		
		std::vector<Menu> mWindowMenus;
	};

}