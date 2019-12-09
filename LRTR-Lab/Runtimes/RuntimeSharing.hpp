#pragma once

#include "../Core/Noncopyable.hpp"

#include <CodeRed/Interface/GpuLogicalDevice.hpp>

#include <memory>

namespace LRTR {

	class UIManager;
	class SceneManager;
	class AssetManager;
	class LabApp;
	
	class RuntimeSharing : public Noncopyable {
	public:
		explicit RuntimeSharing(LabApp* labApp);

		auto uiManager() const noexcept -> std::shared_ptr<UIManager>;

		auto sceneManager() const noexcept -> std::shared_ptr<SceneManager>;

		auto assetManager() const noexcept -> std::shared_ptr<AssetManager>;

		auto allocator() const noexcept -> std::shared_ptr<CodeRed::GpuCommandAllocator>;
		
		auto device() const noexcept -> std::shared_ptr<CodeRed::GpuLogicalDevice>;
		
		auto queue() const noexcept -> std::shared_ptr<CodeRed::GpuCommandQueue>;
	private:
		LabApp* mLabApp;
	};
	
}