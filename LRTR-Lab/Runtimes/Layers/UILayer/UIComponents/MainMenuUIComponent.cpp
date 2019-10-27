#include "MainMenuUIComponent.hpp"

LRTR::MainMenuUIComponent::MainMenuUIComponent()
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&MainMenuUIComponent::updateUI, this));
}

void LRTR::MainMenuUIComponent::updateUI()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		
		ImGui::MenuItem("New");
		ImGui::MenuItem("Open");
		
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}
