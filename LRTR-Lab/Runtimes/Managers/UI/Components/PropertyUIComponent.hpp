#pragma once

#include "../../../../Core/Propertyable.hpp"

#include "UIComponent.hpp"

namespace LRTR {

	class PropertyUIComponent : public UIComponent {
	public:
		explicit PropertyUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~PropertyUIComponent() = default;

		void showProperty(const std::shared_ptr<Propertyable>& property);
	private:
		void update();
	private:
		std::weak_ptr<Propertyable> mProperty;
	};
	
}