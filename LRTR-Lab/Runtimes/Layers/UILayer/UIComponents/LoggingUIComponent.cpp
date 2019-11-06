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
	
	ImGui::Begin("Logging", &mShow);

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
