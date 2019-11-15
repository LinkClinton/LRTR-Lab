#include "LoggingUIComponent.hpp"

#include "../../../../Extensions/SpdLog/SinkStorage.hpp"

#include "../UIManager.hpp"

LRTR::LoggingUIComponent::LoggingUIComponent(const std::shared_ptr<RuntimeSharing>& sharing) :
	UIComponent(sharing)
{
	mImGuiView = std::make_shared<CodeRed::ImGuiView>(
		std::bind(&LoggingUIComponent::update, this));
}

void LRTR::LoggingUIComponent::update()
{
	if (mShow == false) return;

	static const auto messageStorage =
		std::static_pointer_cast<SinkStorageSingleThread>(spdlog::default_logger()->sinks()[1]);

	static auto imGuiWindowFlags = 
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoCollapse;
	
	ImGui::Begin("Logging", &mShow, imGuiWindowFlags);

	const auto sceneViewPosition = mRuntimeSharing->uiManager()->components().at("View.Scene")->position();
	const auto sceneViewSize = mRuntimeSharing->uiManager()->components().at("View.Scene")->size();
	
	//current version of logging window, we do not save the position and the size
	//when we run the program, the size and position of logging window will be reset.
	ImGui::SetWindowSize(ImVec2(
		sceneViewSize.x,
		mRuntimeSharing->uiManager()->height() - sceneViewPosition.y - sceneViewSize.y));

	ImGui::SetWindowPos(ImVec2(
		0,
		sceneViewPosition.y + sceneViewSize.y));
	
	ImGui::BeginChild("View.Logging.Scroll");

	for (const auto message : messageStorage->messages()) {
		std::string subMessages[3] = {
			message.Message.substr(0, message.ColorRange.first - 0),
			message.Message.substr(message.ColorRange.first, message.ColorRange.second - message.ColorRange.first),
			message.Message.substr(message.ColorRange.second, message.Message.size() - message.ColorRange.second)
		};

		ImGui::TextWrapped(subMessages[0].c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(
			message.Color.Red,
			message.Color.Green,
			message.Color.Blue,
			message.Color.Alpha
		));
		ImGui::SameLine(0, 0);
		ImGui::TextWrapped(subMessages[1].c_str());
		ImGui::PopStyleColor();
		ImGui::SameLine(0, 0);
		ImGui::TextWrapped(subMessages[2].c_str());
	}
	
	ImGui::EndChild();

	updateProperties();
	
	ImGui::End();
}
