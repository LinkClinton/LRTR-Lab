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

		auto view() const noexcept -> std::shared_ptr<CodeRed::ImGuiView> { return mImGuiView; }
	protected:
		bool mShow = true;

		std::shared_ptr<CodeRed::ImGuiView> mImGuiView;

		std::shared_ptr<RuntimeSharing> mRuntimeSharing;
	};

}