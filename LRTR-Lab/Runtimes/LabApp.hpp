#pragma once

#include "../Core/Noncopyable.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include <Windows.h>

#include <string>
#include <memory>

namespace LRTR {

	class UILayer;

	class LabApp final : public Noncopyable {
	public:
		explicit LabApp(
			const std::string& name,
			const size_t width,
			const size_t height);

		~LabApp();

		void show() const;

		void hide() const;

		void runLoop();

		auto width() const noexcept -> size_t { return mWidth; }

		auto height() const noexcept -> size_t { return mHeight; }

		auto name() const noexcept -> std::string { return mName; }

		auto handle() const noexcept -> void* { return mHwnd; }
	private:
		void initialize();

		void update(float delta);
		void render(float delta);
	private:
		size_t mCurrentFrameIndex = 0;

		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		
		std::vector<std::shared_ptr<CodeRed::GpuFrameBuffer>> mFrameBuffers;

		std::shared_ptr<CodeRed::GpuCommandAllocator> mCommandAllocator;
		std::shared_ptr<CodeRed::GpuCommandQueue> mCommandQueue;

		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;
		std::shared_ptr<CodeRed::GpuSwapChain> mSwapChain;
		
		void initializeDevice();

		void initializeCommand();

		void initializeSwapChain();
	private:
		std::shared_ptr<UILayer> mUILayer;

		void initializeUILayer();
	private:
		std::string mName;

		size_t mWidth;
		size_t mHeight;

		HWND mHwnd;

		bool mExisted;

		static void processMessage(LabApp* app, const MSG& message);
	};

}