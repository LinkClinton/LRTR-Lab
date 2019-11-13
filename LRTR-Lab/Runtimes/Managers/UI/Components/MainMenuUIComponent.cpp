#include "MainMenuUIComponent.hpp"

#include "../UIManager.hpp"

LRTR::MainMenuUIComponent::MainMenuUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&MainMenuUIComponent::update, this));

	mWindowMenus.insert({ "View", Menu() });
	//mWindowMenus.insert({ "Manager",Menu() });

	mWindowMenus["View"].insert({ "Logging", "View.Logging" });
	mWindowMenus["View"].insert({ "Scene", "View.Scene" });
	mWindowMenus["View"].insert({ "Shape", "View.Shape" });
	//mWindowMenus["View"].insert({ "Manager", "View.Manager" });

	//mWindowMenus["Manager"].insert({ "Scene", "Manager.SceneManager" });
	//mWindowMenus["Manager"].insert({ "UI","Manager.UIManager" });
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

	for (const auto &menu : mWindowMenus) {
		if (ImGui::BeginMenu(menu.first.c_str())) {

			for (const auto& item : menu.second) {
				if (ImGui::MenuItem(item.first.c_str())) {
					mRuntimeSharing->uiManager()->components().at(item.second)->show();
				}
			}

			ImGui::EndMenu();
		}
	}

	mSize = ImGui::GetWindowSize();
	
	ImGui::EndMainMenuBar();
}
