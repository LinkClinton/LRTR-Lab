#include "ConsoleUIComponent.hpp"

#include "../UIManager.hpp"

LRTR::ConsoleUIComponent::ConsoleUIComponent(const std::shared_ptr<RuntimeSharing>& sharing)
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

	updateProperties();
	
	ImGui::End();
}
