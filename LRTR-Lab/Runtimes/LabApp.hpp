#pragma once

#include "../Core/Noncopyable.hpp"
#include "../Shared/Math/Math.hpp"

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include <Windows.h>

#include <string>
#include <memory>

namespace LRTR {

	class RuntimeSharing;
	class SceneManager;
	class AssetManager;
	class InputManager;
	class UIManager;

	struct AppStartup {
		std::string Name = "LabApp";
		size_t Width = 1920;
		size_t Height = 1080;
		size_t Font = 20;

		AppStartup() = default;

		AppStartup(
			const std::string& name,
			const size_t width,
			const size_t height,
			const size_t font) :
			Name(name), Width(width), Height(height), Font(font) {}
	};
	
	class LabApp final : public Noncopyable {
	public:
		explicit LabApp(const AppStartup& startup);

		~LabApp();

		void show() const;

		void hide() const;

		void runLoop();

		auto width() const noexcept -> size_t { return mStartup.Width; }

		auto height() const noexcept -> size_t { return mStartup.Height; }

		auto name() const noexcept -> std::string { return mStartup.Name; }

		auto handle() const noexcept -> void* { return mHwnd; }
	private:
		void initializeLogComponents();

		void initializeCodeRedComponents();

		void initializeManagerComponents();
		
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
		std::shared_ptr<SceneManager> mSceneManager;
		std::shared_ptr<AssetManager> mAssetManager;
		std::shared_ptr<InputManager> mInputManager;
		std::shared_ptr<UIManager> mUIManager;

		void initializeSceneManager();

		void initializeAssetManager();

		void initializeInputManager();
		
		void initializeUIManager();
	private:
		friend class RuntimeSharing;
		
		std::shared_ptr<RuntimeSharing> mRuntimeSharing;
		
		AppStartup mStartup;
		
		HWND mHwnd;

		bool mExisted;

		Vector2f mMousePosition;

		static void processMessage(LabApp* app, const MSG& message);
	};

}