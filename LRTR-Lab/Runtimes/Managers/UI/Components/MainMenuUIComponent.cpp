#include "MainMenuUIComponent.hpp"

#include "../UIManager.hpp"

LRTR::MainMenuUIComponent::MainMenuUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
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
			mRuntimeSharing->uiManager()->components().at("View.Console")->show();

		if (ImGui::MenuItem("Logging"))
			mRuntimeSharing->uiManager()->components().at("View.Logging")->show();

		if (ImGui::MenuItem("Scene"))
			mRuntimeSharing->uiManager()->components().at("View.Scene")->show();
		
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}
