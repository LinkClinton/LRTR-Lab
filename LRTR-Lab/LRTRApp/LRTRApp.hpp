#pragma once

#include <Code-Red/CodeRed/Core/CodeRedGraphics.hpp>

#include <Windows.h>

#include <string>
#include <chrono>

namespace LRTR {

	using Time = std::chrono::high_resolution_clock;

	class LRTRApp final : public CodeRed::Noncopyable {
	public:
		explicit LRTRApp(
			const std::string& name,
			const size_t width,
			const size_t height);

		~LRTRApp();

		void show() const;

		void hide() const;

		void runLoop();

		auto width() const noexcept -> size_t { return mWidth; }

		auto height() const noexcept -> size_t { return mHeight; }

		auto name() const noexcept -> std::string { return mName; }

		auto handle() const noexcept -> void* { return mHwnd; }
	private:
		void update(float delta);
		void render(float delta);
	private:
		std::string mName;

		size_t mWidth;
		size_t mHeight;
		HWND mHwnd;

		bool mExisted;

		static void processMessage(LRTRApp* app, const MSG& message);
	};

}