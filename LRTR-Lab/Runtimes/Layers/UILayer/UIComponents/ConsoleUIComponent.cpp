#include "ConsoleUIComponent.hpp"

#include "../UILayer.hpp"

LRTR::ConsoleUIComponent::ConsoleUIComponent(const std::shared_ptr<UILayerSharing>& sharing)
	: UIComponent(sharing)
{
	mShow = false;
	
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&ConsoleUIComponent::update, this));
}

void LRTR::ConsoleUIComponent::update()
{
	if (mShow == false) return;
	
	ImGui::Begin("Console", &mShow);


	
	ImGui::End();
}
