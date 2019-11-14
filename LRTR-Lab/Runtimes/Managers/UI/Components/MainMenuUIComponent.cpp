#include "MainMenuUIComponent.hpp"

#include "../UIManager.hpp"

LRTR::MainMenuUIComponent::MainMenuUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&MainMenuUIComponent::update, this));

	initializeWindowsMenus();

	mWindowMenus[nameIndex("View")].second.push_back({ "Property", "View.Property" });
	mWindowMenus[nameIndex("View")].second.push_back({ "Logging", "View.Logging" });
	mWindowMenus[nameIndex("View")].second.push_back({ "Scene", "View.Scene" });
	mWindowMenus[nameIndex("View")].second.push_back({ "Shape", "View.Shape" });
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

	updateProperties();
	
	ImGui::EndMainMenuBar();
}

auto LRTR::MainMenuUIComponent::nameIndex(const std::string& name) const -> size_t
{
	return mNameIndices.at(name);
}

void LRTR::MainMenuUIComponent::initializeWindowsMenus()
{
	mWindowMenus.push_back({ "View", Components() });
	//mWindowMenus.insert({ "Manager",Menu() });

	for (size_t index = 0; index < mWindowMenus.size(); index++)
		mNameIndices.insert({ mWindowMenus[index].first, index });
}
