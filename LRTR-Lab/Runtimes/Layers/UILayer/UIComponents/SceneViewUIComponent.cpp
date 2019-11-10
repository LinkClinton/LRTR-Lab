#include "SceneViewUIComponent.hpp"

#include "../../../../Scenes/Scene.hpp"
#include "../UILayer.hpp"

LRTR::SceneViewUIComponent::SceneViewUIComponent(const std::shared_ptr<UILayerSharing>& sharing) :
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

	//the scene is only used for test for current version
	static auto scene = std::make_shared<Scene>(
		mLayerSharing->device(),
		mLayerSharing->commandAllocator(),
		mLayerSharing->commandQueue());

	ImGui::Begin("Scene", &mShow, imGuiWindowFlags);

	const auto contentSize = ImVec2(
		ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
		ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y
	);
	
	if (texture == nullptr || 
		texture->width() != static_cast<size_t>(contentSize.x) ||
		texture->height() != static_cast<size_t>(contentSize.y)) {
		
		texture = mLayerSharing->device()->createTexture(
			CodeRed::ResourceInfo::Texture2D(
				static_cast<size_t>(contentSize.x),
				static_cast<size_t>(contentSize.y),
				CodeRed::PixelFormat::RedGreenBlueAlpha8BitUnknown,
				CodeRed::ResourceUsage::RenderTarget)
		);

		scene->setTarget(texture);
	}

	ImGui::Image(scene->generate(nullptr).get(), contentSize);
	
	ImGui::End();
}
