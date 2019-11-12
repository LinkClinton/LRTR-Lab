#pragma once

#include "../UIComponent.hpp"

namespace LRTR {

	class SceneManagerUIComponent : public ContentUIComponent {
	public:
		explicit SceneManagerUIComponent(const std::shared_ptr<RuntimeSharing>& sharing);

		~SceneManagerUIComponent() = default;

		void content() override;

		auto selected() const noexcept -> std::string;
	private:
		void update();
	private:
		std::string mSelected;
	};
}