#pragma once

#include <CodeRed/Core/CodeRedGraphics.hpp>

#include "../Shared/Accelerators/Group.hpp"
#include "../Core/Noncopyable.hpp"
#include "Cameras/Camera.hpp"
#include "System.hpp"
#include "Shape.hpp"

#include <unordered_map>
#include <memory>

namespace LRTR {

	using SceneCamera = ProjectiveCamera;

	class Scene : public Noncopyable {
	public:
		explicit Scene(
			const std::string& name,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			const size_t maxFrameCount = 2);

		virtual ~Scene() = default;

		void add(const std::shared_ptr<Shape>& shape);

		void addSystem(const std::shared_ptr<System>& system);

		void remove(const Identity& identity);

		auto name() const noexcept -> std::string;

		auto shapes() const noexcept -> const Group<Identity, std::shared_ptr<Shape>>&;

		auto systems() const noexcept -> const std::vector<std::shared_ptr<System>>&;

		auto property() const noexcept -> std::shared_ptr<Shape>;
		
		auto currentFrameIndex() const noexcept -> size_t;
	protected:
		virtual void update(float delta);

		virtual auto render(
			const std::shared_ptr<CodeRed::GpuTexture>& texture,
			const std::shared_ptr<SceneCamera>& camera,
			float delta)
			-> std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>;
	private:
		void setTarget(const std::shared_ptr<CodeRed::GpuTexture>& texture);

		friend class SceneManager;
		friend class LabApp;
	protected:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;

		std::shared_ptr<CodeRed::GpuFrameBuffer> mFrameBuffer;
		std::shared_ptr<CodeRed::GpuRenderPass> mRenderPass;

		std::shared_ptr<CodeRed::GpuTexture> mBlurTexture;
		std::shared_ptr<CodeRed::GpuTexture> mDepthStencil;

		std::vector<std::shared_ptr<CodeRed::GpuCommandAllocator>> mCommandAllocators;
		std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>> mCommandLists;
	private:
		size_t mCurrentFrameIndex = 0;
		size_t mMaxFrameCount = 0;
		
		std::string mName;

		std::vector<std::shared_ptr<System>> mSystems;
		
		Group<Identity, std::shared_ptr<Shape>> mShapes;

		std::shared_ptr<Shape> mProperty;
	};
	
}