#pragma once

#include "../../../../../Shared/Accelerators/Group.hpp"
#include "../UIComponent.hpp"

namespace LRTR {

	class ManagerUIComponent : public UIComponent {
	public:
		explicit ManagerUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~ManagerUIComponent() = default;
	private:
		void update();
	private:
		StringGroup<std::string> mMangerUIComponents;
	};
	
}