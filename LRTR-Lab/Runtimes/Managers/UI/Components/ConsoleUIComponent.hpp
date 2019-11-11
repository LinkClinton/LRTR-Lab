#pragma once

#include "UIComponent.hpp"

namespace LRTR {

	class ConsoleUIComponent : public UIComponent {
	public:
		explicit ConsoleUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~ConsoleUIComponent() = default;
	private:
		void update();
	};
	
}