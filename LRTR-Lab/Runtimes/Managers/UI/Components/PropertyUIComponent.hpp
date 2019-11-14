#pragma once

#include "UIComponent.hpp"

namespace LRTR {

	class PropertyUIComponent : public UIComponent {
	public:
		explicit PropertyUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~PropertyUIComponent() = default;
	private:
		void update();
	};
	
}