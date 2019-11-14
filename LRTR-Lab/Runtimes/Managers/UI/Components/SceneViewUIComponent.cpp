#include "SceneViewUIComponent.hpp"

#include "../UIManager.hpp"

LRTR::SceneViewUIComponent::SceneViewUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&SceneViewUIComponent::update, this));
}

auto LRTR::SceneViewUIComponent::sceneTexture() const noexcept -> std::shared_ptr<CodeRed::GpuTexture>
{
	return mSceneTexture;
}

void LRTR::SceneViewUIComponent::update()
{
	if (mShow == false) return;

	static auto imGuiWindowFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("Scene", &mShow, imGuiWindowFlags);

	const auto mainMenuHeight = mRuntimeSharing->uiManager()->components().at("MainMenu")->size().y;
	
	ImGui::SetWindowSize(ImVec2(
		mRuntimeSharing->uiManager()->width() * 0.8f,
		mRuntimeSharing->uiManager()->height() * 0.8f
	));

	ImGui::SetWindowPos(ImVec2(
		0,
		mainMenuHeight
	));
	
	const auto contentSize = ImVec2(
		std::max(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, 1.f),
		std::max(ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y, 1.f)
	);

	if (mSceneTexture == nullptr ||
		mSceneTexture->width() != static_cast<size_t>(contentSize.x) ||
		mSceneTexture->height() != static_cast<size_t>(contentSize.y)) {

		mSceneTexture = mRuntimeSharing->device()->createTexture(
			CodeRed::ResourceInfo::RenderTarget(
				static_cast<size_t>(contentSize.x),
				static_cast<size_t>(contentSize.y),
				CodeRed::PixelFormat::RedGreenBlueAlpha8BitUnknown,
				CodeRed::ClearValue(0.227450f, 0.227450f, 0.227450f, 1)));
	}

	ImGui::Image(mSceneTexture.get(), contentSize);

	updateProperties();
	
	ImGui::End();
}
