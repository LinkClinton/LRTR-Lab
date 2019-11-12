#pragma once

#include "../UIComponent.hpp"

namespace LRTR {

	class UIManagerUIComponent : public ContentUIComponent {
	public:
		explicit UIManagerUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~UIManagerUIComponent() = default;
		
		void content() override;
	private:
		void update();
	private:
		std::string mSelected = "";
	};
	
}