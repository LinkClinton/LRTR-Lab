#pragma once

#include "UIComponent.hpp"

namespace LRTR {

	class LoggingUIComponent : public UIComponent {
	public:
		explicit LoggingUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~LoggingUIComponent() = default;
	private:
		void update();
	};
	
}