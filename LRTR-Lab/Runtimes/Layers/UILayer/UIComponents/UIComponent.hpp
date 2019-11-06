#pragma once

#include "../../../../Core/Noncopyable.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

#include <memory>

namespace LRTR {

	class UILayerSharing;
	
	class UIComponent : public Noncopyable {
	public:
		explicit UIComponent(
			const std::shared_ptr<UILayerSharing> &sharing) : mLayerSharing(sharing) {}

		~UIComponent() = default;

		virtual void show() noexcept { mShow = true; }

		virtual void hide() noexcept { mShow = false; }
		
		auto view() const noexcept -> std::shared_ptr<CodeRed::ImGuiView> { return mImGuiView; }
	protected:
		bool mShow = true;
		
		std::shared_ptr<CodeRed::ImGuiView> mImGuiView;

		std::shared_ptr<UILayerSharing> mLayerSharing;
	};

}