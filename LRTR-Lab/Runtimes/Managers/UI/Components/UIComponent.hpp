#pragma once

#include "../../../../Core/Noncopyable.hpp"
#include "../../../RuntimeSharing.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

#include <memory>

namespace LRTR {

	class UIComponent : public Noncopyable {
	public:
		explicit UIComponent(
			const std::shared_ptr<RuntimeSharing>& sharing) : mRuntimeSharing(sharing) {}

		~UIComponent() = default;

		virtual void show() noexcept { mShow = true; }

		virtual void hide() noexcept { mShow = false; }

		auto status() const noexcept -> bool { return mShow; }
		
		auto view() const noexcept -> std::shared_ptr<CodeRed::ImGuiView> { return mImGuiView; }

		auto size() const noexcept -> ImVec2 { return mSize; }

		auto position() const noexcept -> ImVec2 { return mPosition; }
	protected:
		virtual void updateProperties()
		{
			mPosition = ImGui::GetWindowPos();
			mSize = ImGui::GetWindowSize();
		}
	protected:
		bool mShow = true;

		ImVec2 mPosition;
		ImVec2 mSize;
		
		std::shared_ptr<CodeRed::ImGuiView> mImGuiView;

		std::shared_ptr<RuntimeSharing> mRuntimeSharing;
	};

	class ContentUIComponent : public UIComponent {
	public:
		explicit ContentUIComponent(
			const std::shared_ptr<RuntimeSharing>& sharing) : UIComponent(sharing) {}

		virtual void content() {}
	};
}