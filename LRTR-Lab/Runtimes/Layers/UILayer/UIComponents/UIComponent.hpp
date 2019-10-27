#pragma once

#include "../../../../Core/Noncopyable.hpp"

#include <Extensions/ImGui/ImGuiWindows.hpp>

#include <memory>

namespace LRTR {

	class UIComponent : public Noncopyable {
	public:
		UIComponent() = default;

		explicit UIComponent(const std::shared_ptr<CodeRed::ImGuiView>& view) :
			mImGuiView(view) {}

		~UIComponent() = default;

		auto view() const noexcept -> std::shared_ptr<CodeRed::ImGuiView> { return mImGuiView; }
	protected:
		std::shared_ptr<CodeRed::ImGuiView> mImGuiView;
	};

}