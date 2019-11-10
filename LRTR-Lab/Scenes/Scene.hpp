#pragma once

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../Shared/Accelerators/Group.hpp"
#include "../Core/Noncopyable.hpp"
#include "Cameras/Camera.hpp"

#include <unordered_map>
#include <memory>

namespace LRTR {

	using SceneCamera = ProjectiveCamera;

	class Scene : public Noncopyable {
	public:
		explicit Scene(
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			const std::shared_ptr<CodeRed::GpuCommandAllocator>& allocator,
			const std::shared_ptr<CodeRed::GpuCommandQueue>& queue);

		virtual ~Scene() = default;

		void setTarget(const std::shared_ptr<CodeRed::GpuTexture>& texture);
		
		auto generate(const std::shared_ptr<SceneCamera>& camera) -> std::shared_ptr<CodeRed::GpuTexture>;
		
		auto cameras() noexcept -> StringGroup<std::shared_ptr<SceneCamera>>&;
	private:
		virtual void update(float delta);
	private:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> mCommandList;
		std::shared_ptr<CodeRed::GpuCommandAllocator> mCommandAllocator;
		std::shared_ptr<CodeRed::GpuCommandQueue> mCommandQueue;

		std::shared_ptr<CodeRed::GpuFrameBuffer> mFrameBuffer;
		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;
		
		StringGroup<std::shared_ptr<SceneCamera>> mCameras;
	};
	
}