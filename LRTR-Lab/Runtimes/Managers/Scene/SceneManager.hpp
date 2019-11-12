#pragma once

#include "../../../Shared/Accelerators/Group.hpp"
#include "../Manager.hpp"

#include <CodeRed/Interface/GpuGraphicsCommandList.hpp>

namespace LRTR {

	class Scene;
	
	class SceneManager : public Manager {
	public:
		explicit SceneManager(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator);

		void update(float delta) override;

		auto render(float delta) -> std::shared_ptr<CodeRed::GpuGraphicsCommandList>;
			
		void add(
			const std::shared_ptr<Scene>& scene);

		void remove(const std::string& name);

		auto scenes() const noexcept -> const StringGroup<std::shared_ptr<Scene>>&;
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;

		std::shared_ptr<CodeRed::GpuCommandAllocator> mCommandAllocator;
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> mCommandList;
		
		StringGroup<std::shared_ptr<Scene>> mScenes;
	};
	
}