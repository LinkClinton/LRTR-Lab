#pragma once

namespace LRTR {

	class Propertyable {
	public:
		Propertyable() = default;

		virtual ~Propertyable() = default;
	protected:
		friend class PropertyUIComponent;
		
		virtual void onProperty() = 0;
	};
	
}