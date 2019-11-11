#include "SceneViewUIComponent.hpp"

#include "../UIManager.hpp"

LRTR::SceneViewUIComponent::SceneViewUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&SceneViewUIComponent::update, this));
}

void LRTR::SceneViewUIComponent::update()
{
	if (mShow == false) return;

	static auto imGuiWindowFlags =
		ImGuiWindowFlags_NoCollapse;

	static std::shared_ptr<CodeRed::GpuTexture> texture = nullptr;

	ImGui::Begin("Scene", &mShow, imGuiWindowFlags);

	const auto contentSize = ImVec2(
		ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
		ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y
	);
	
	if (texture == nullptr || 
		texture->width() != static_cast<size_t>(contentSize.x) ||
		texture->height() != static_cast<size_t>(contentSize.y)) {

		texture = mRuntimeSharing->device()->createTexture(
			CodeRed::ResourceInfo::RenderTarget(
				static_cast<size_t>(contentSize.x),
				static_cast<size_t>(contentSize.y),
				CodeRed::PixelFormat::RedGreenBlueAlpha8BitUnknown,
				CodeRed::ClearValue(1, 0, 0, 1)));
	}

	ImGui::Image(texture.get(), contentSize);
	
	ImGui::End();
}
