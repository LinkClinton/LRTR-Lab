#include "SceneShapeUIComponent.hpp"

#include "../UIManager.hpp"

LRTR::SceneShapeUIComponent::SceneShapeUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&SceneShapeUIComponent::update, this));
}

void LRTR::SceneShapeUIComponent::update()
{
	if (mShow == false) return;

	static auto imGuiWindowFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;
	
	ImGui::Begin("Shape", &mShow, imGuiWindowFlags);

	const auto mainMenuHeight = mRuntimeSharing->uiManager()->components().at("MainMenu")->size().y;

	ImGui::SetWindowSize(ImVec2(
		mRuntimeSharing->uiManager()->width() * 0.20f,
		mRuntimeSharing->uiManager()->height() * (1.0f - 0.6f) - mainMenuHeight
	));

	ImGui::SetWindowPos(ImVec2(
		mRuntimeSharing->uiManager()->width() * (1.0f - 0.20f),
		mainMenuHeight
	));

	updateProperties();
	
	ImGui::End();
}
