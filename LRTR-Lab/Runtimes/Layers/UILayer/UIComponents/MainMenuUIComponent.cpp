#include "MainMenuUIComponent.hpp"

#include "../UILayer.hpp"

LRTR::MainMenuUIComponent::MainMenuUIComponent(const std::shared_ptr<UILayerSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&MainMenuUIComponent::update, this));
}

void LRTR::MainMenuUIComponent::update()
{
	if (mShow == false) return;
	
	ImGui::BeginMainMenuBar();
	
	if (ImGui::BeginMenu("File")) {
		
		ImGui::MenuItem("New");
		ImGui::MenuItem("Open");
		
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View")) {

		if (ImGui::MenuItem("Console")) 
			mLayerSharing->components().at("View.Console")->show();

		if (ImGui::MenuItem("Logging"))
			mLayerSharing->components().at("View.Logging")->show();

		if (ImGui::MenuItem("Scene"))
			mLayerSharing->components().at("View.Scene")->show();
		
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}
