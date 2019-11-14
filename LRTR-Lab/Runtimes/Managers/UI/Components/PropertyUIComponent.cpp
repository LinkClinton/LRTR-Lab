#include "PropertyUIComponent.hpp"

#include "../UIManager.hpp"

LRTR::PropertyUIComponent::PropertyUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&PropertyUIComponent::update, this));
}

void LRTR::PropertyUIComponent::update()
{
	if (mShow == false) return;

	if (mShow == false) return;

	static auto imGuiWindowFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("Property", &mShow, imGuiWindowFlags);

	const auto sceneShapePosition = mRuntimeSharing->uiManager()->components().at("View.Shape")->position();
	const auto sceneShapeSize = mRuntimeSharing->uiManager()->components().at("View.Shape")->size();
	
	ImGui::SetWindowSize(ImVec2(
		sceneShapeSize.x,
		mRuntimeSharing->uiManager()->height() - sceneShapePosition.y - sceneShapeSize.y
	));

	ImGui::SetWindowPos(ImVec2(
		sceneShapePosition.x, 
		sceneShapePosition.y + sceneShapeSize.y
	));

	updateProperties();

	ImGui::End();
}
