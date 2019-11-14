#pragma once

namespace LRTR {

	class Propertyable {
	protected:
		friend class PropertyUIComponent;
		
		virtual void onProperty() = 0;
	};
	
}