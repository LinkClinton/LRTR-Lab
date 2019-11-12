#include "SceneManagerUIComponent.hpp"

#include "../../../Scene/SceneManager.hpp"

LRTR::SceneManagerUIComponent::SceneManagerUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	ContentUIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&SceneManagerUIComponent::update, this));
}

void LRTR::SceneManagerUIComponent::content()
{
	for (auto scene : mRuntimeSharing->sceneManager()->scenes()) {
		const auto status = mSelected == scene.first ? true : false;

		if (ImGui::Selectable(scene.first.c_str(), status,
			ImGuiSelectableFlags_AllowDoubleClick)) {

			if (ImGui::IsMouseDoubleClicked(0)) {
				mSelected = scene.first;
			}
		}
	}
}

auto LRTR::SceneManagerUIComponent::selected() const noexcept -> std::string
{
	return mSelected;
}

void LRTR::SceneManagerUIComponent::update()
{
	if (mShow == false) return;
}
