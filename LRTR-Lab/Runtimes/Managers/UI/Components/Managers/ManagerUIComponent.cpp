#include "ManagerUIComponent.hpp"

#include "UIManagerUIComponent.hpp"

#include "../../UIManager.hpp"

LRTR::ManagerUIComponent::ManagerUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&ManagerUIComponent::update, this));
	
	mMangerUIComponents.insert({ "Scene", "Manager.SceneManager" });
	mMangerUIComponents.insert({ "UI", "Manager.UIManager" });
}

void LRTR::ManagerUIComponent::update()
{
	if (mShow == false) return;

	static auto imGuiWindowFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("Manager", &mShow, imGuiWindowFlags);

	const auto mainMenuHeight = mRuntimeSharing->uiManager()->components().at("MainMenu")->size().y;
	
	ImGui::SetWindowSize(ImVec2(
		mRuntimeSharing->uiManager()->width() * 0.2f,
		mRuntimeSharing->uiManager()->height() * 1.0f - mainMenuHeight
	));

	ImGui::SetWindowPos(ImVec2(
		mRuntimeSharing->uiManager()->width() * (1.0f - 0.2f),
		mainMenuHeight
	));

	StringGroup<std::string> showUIComponents;
	
	for (auto component : mMangerUIComponents) {
		if (mRuntimeSharing->uiManager()->components().at(component.second)->status())
			showUIComponents.insert(component);
	}

	//if the component is not empty, we will draw the content
	if (!showUIComponents.empty()) {
		ImGui::BeginTabBar("ManagerTabBar");

		for (auto component : showUIComponents) {
			auto status = true;

			//the two space is used to expand the size of tab
			//because the origin size of tab is too small to close easily
			if (ImGui::BeginTabItem((component.first + "  ").c_str(), &status)) {
				std::static_pointer_cast<ContentUIComponent>(
					mRuntimeSharing->uiManager()->components().at(component.second))
					->content();
				
				ImGui::EndTabItem();
			}

			//when we close the tab item, we need hide the ui component
			if (status == false)
				mRuntimeSharing->uiManager()->components().at(component.second)->hide();
		}
		
		ImGui::EndTabBar();
	}
	
	updateProperties();
	
	ImGui::End();
}
