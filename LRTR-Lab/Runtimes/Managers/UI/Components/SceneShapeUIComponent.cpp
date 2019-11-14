#include "SceneShapeUIComponent.hpp"

#include "../../../../Scenes/Scene.hpp"
#include "../../Scene/SceneManager.hpp"
#include "../UIManager.hpp"

#include "PropertyUIComponent.hpp"

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

	const auto sceneViewWidth = mRuntimeSharing->uiManager()->components().at("View.Scene")->size().x;
	const auto mainMenuHeight = mRuntimeSharing->uiManager()->components().at("MainMenu")->size().y;

	ImGui::SetWindowSize(ImVec2(
		mRuntimeSharing->uiManager()->width() - sceneViewWidth,
		mRuntimeSharing->uiManager()->height() * (1.0f - 0.6f) - mainMenuHeight
	));

	ImGui::SetWindowPos(ImVec2(
		sceneViewWidth,
		mainMenuHeight
	));

	for (auto shape : mRuntimeSharing->sceneManager()->scenes().at("Scene")->shapes()) {
		const auto status = mSelected == shape.first ? true : false;

		if (ImGui::Selectable(shape.first.c_str(), status)) {
			mSelected = shape.first;

			std::static_pointer_cast<PropertyUIComponent>(
				mRuntimeSharing->uiManager()->components().at("View.Property"))
				->showProperty(shape.second);
			
		}
	}
	
	updateProperties();
	
	ImGui::End();
}
