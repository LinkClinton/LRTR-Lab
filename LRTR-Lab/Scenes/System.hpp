#pragma once

#include "../Runtimes/RuntimeSharing.hpp"
#include "../Shared/FrameResources.hpp"
#include "../Core/Noncopyable.hpp"

#include "Cameras/Camera.hpp"
#include "Shape.hpp"

#include <CodeRed/Interface/GpuGraphicsCommandList.hpp>

#define SHADER_SOURCE_HLSL
#ifndef SHADER_SOURCE_HLSL
#define SHADER_SOURCE_GLSL
#endif

namespace LRTR {

	using SceneCamera = ProjectiveCamera;
	
	class System : public Noncopyable, public TypeInfo {
	public:
		explicit System(const std::shared_ptr<RuntimeSharing>& sharing);

		~System() = default;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		std::shared_ptr<RuntimeSharing> mRuntimeSharing;
	};

	class UpdateSystem : public System {
	public:
		explicit UpdateSystem(const std::shared_ptr<RuntimeSharing>& sharing);

		~UpdateSystem() = default;

		virtual void update(
			const Group<Identity, std::shared_ptr<Shape>>& shapes,
			float delta) = 0;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	};

	class RenderSystem : public UpdateSystem {
	public:
		explicit RenderSystem(
			const std::shared_ptr<RuntimeSharing>& sharing,
			const std::shared_ptr<CodeRed::GpuLogicalDevice>& device,
			size_t maxFrameCount = 2);

		~RenderSystem() = default;
	
		virtual void render(
			const std::vector<std::shared_ptr<CodeRed::GpuGraphicsCommandList>>& commandLists,
			const std::shared_ptr<CodeRed::GpuFrameBuffer>& frameBuffer,
			const std::shared_ptr<SceneCamera>& camera,
			float delta) = 0;

		auto typeName() const noexcept -> std::string override;

		auto typeIndex() const noexcept -> std::type_index override;
	protected:
		std::shared_ptr<CodeRed::GpuLogicalDevice> mDevice;

		std::vector<FrameResources> mFrameResources;
		
		size_t mCurrentFrameIndex = 0;
	};
	
}