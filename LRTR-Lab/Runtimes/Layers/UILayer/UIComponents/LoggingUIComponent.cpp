#include "LoggingUIComponent.hpp"

#include "../../../../Extensions/SpdLog/SinkStorage.hpp"

#include "../UILayer.hpp"

LRTR::LoggingUIComponent::LoggingUIComponent(const std::shared_ptr<UILayerSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&LoggingUIComponent::update, this));
}

void LRTR::LoggingUIComponent::update()
{
	if (mShow == false) return;

	static const auto messageStorage =
		std::static_pointer_cast<SinkStorageSingleThread>(spdlog::default_logger()->sinks()[0]);

	static auto imGuiWindowFlags = 
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoCollapse;
	
	ImGui::Begin("Logging", &mShow, imGuiWindowFlags);

	//current version of logging window, we do not save the position and the size
	//when we run the program, the size and position of logging window will be reset.
	ImGui::SetWindowSize(ImVec2(mLayerSharing->width() * 1.0f, mLayerSharing->height() * 0.25f));
	ImGui::SetWindowPos(ImVec2(0, mLayerSharing->height() * (1.0f - 0.25f)));
	
	ImGui::BeginChild("View.Logging.Scroll");

	for (const auto message : messageStorage->messages()) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(
			message.Color.Red,
			message.Color.Green,
			message.Color.Blue,
			message.Color.Alpha));
		ImGui::TextWrapped(message.Message.c_str());
		ImGui::PopStyleColor();
	}
	
	ImGui::EndChild();
	
	ImGui::End();
}
