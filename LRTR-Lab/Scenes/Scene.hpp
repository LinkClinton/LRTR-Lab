#pragma once

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../Shared/Accelerators/Group.hpp"
#include "../Core/Noncopyable.hpp"
#include "Cameras/Camera.hpp"
#include "Shape.hpp"

#include <unordered_map>
#include <memory>

namespace LRTR {

	using SceneCamera = ProjectiveCamera;

	class Scene : public Noncopyable {
	public:
		explicit Scene(
			const std::string& name,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device);

		virtual ~Scene() = default;

		void add(
			const std::string& name,
			const std::shared_ptr<Shape>& shape);

		void remove(
			const std::string& name);
		
		auto generate(
			const std::shared_ptr<CodeRed::GpuTexture>& texture,
			const std::shared_ptr<SceneCamera>& camera)
			-> std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>;
	
		auto name() const noexcept -> std::string;

		auto shapes() const noexcept -> const StringGroup<std::shared_ptr<Shape>>&;
	private:
		virtual void update(float delta);
		
		void setTarget(const std::shared_ptr<CodeRed::GpuTexture>& texture);
	private:
		std::string mName;
		
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;
		
		std::shared_ptr<CodeRed::GpuGraphicsCommandList> mCommandList;
		std::shared_ptr<CodeRed::GpuCommandAllocator> mCommandAllocator;
		
		std::shared_ptr<CodeRed::GpuFrameBuffer> mFrameBuffer;
		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;

		StringGroup<std::shared_ptr<Shape>> mShapes;
	};
	
}