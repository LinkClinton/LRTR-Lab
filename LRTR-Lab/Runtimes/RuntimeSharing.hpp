#pragma once

#include "../Core/Noncopyable.hpp"

#include <CodeRed/Interface/GpuLogicalDevice.hpp>

#include <memory>

namespace LRTR {

	class UIManager;
	class LabApp;
	
	class RuntimeSharing : public Noncopyable {
	public:
		explicit RuntimeSharing(LabApp* labApp);

		auto uiManager() const noexcept -> std::shared_ptr<UIManager>;

		auto device() const noexcept -> std::shared_ptr<CodeRed::GpuLogicalDevice>;
	private:
		LabApp* mLabApp;
	};
	
}