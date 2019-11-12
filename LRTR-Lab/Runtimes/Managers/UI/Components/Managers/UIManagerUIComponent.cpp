#include "UIManagerUIComponent.hpp"

#include "../../UIManager.hpp"

LRTR::UIManagerUIComponent::UIManagerUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	ContentUIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&UIManagerUIComponent::update, this));
}

void LRTR::UIManagerUIComponent::content()
{
	for (auto component : mRuntimeSharing->uiManager()->components()) {
		const auto status = mSelected == component.first ? true : false;
		
		if (ImGui::Selectable(component.first.c_str(), status, 
			ImGuiSelectableFlags_AllowDoubleClick)) {

			if (ImGui::IsMouseDoubleClicked(0)) {
				mSelected = component.first;
			}
		}
	}
}

void LRTR::UIManagerUIComponent::update()
{
	if (mShow == false) return;
}
