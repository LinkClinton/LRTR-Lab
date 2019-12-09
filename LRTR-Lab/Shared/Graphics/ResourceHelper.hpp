#pragma once

#include <CodeRed/Core/CodeRedGraphics.hpp>

namespace CodeRed {

	class ResourceHelper {
	public:
		static void updateBuffer(
			const std::shared_ptr<GpuLogicalDevice>& device,
			const std::shared_ptr<GpuCommandAllocator> &allocator,
			const std::shared_ptr<GpuBuffer>& buffer,
			const void* data
		);

		static void updateBuffer(
			const std::shared_ptr<GpuLogicalDevice>& device,
			const std::shared_ptr<GpuCommandAllocator>& allocator,
			const std::shared_ptr<GpuCommandQueue>& queue,
			const std::shared_ptr<GpuBuffer>& buffer,
			const void* data
		);

		static void updateBuffer(
			const std::shared_ptr<GpuBuffer>& buffer,
			const void* data,
			const size_t size = 0);

		static void updateBuffer(
			const std::shared_ptr<GpuBuffer>& buffer,
			const void* data,
			const size_t offset,
			const size_t size);

		static void copyBuffer(
			const std::shared_ptr<GpuBuffer>& destination,
			const std::shared_ptr<GpuBuffer>& source,
			const size_t offset = 0);
		
		static auto expandBuffer(
			const std::shared_ptr<GpuLogicalDevice>& device,
			const std::shared_ptr<GpuBuffer>& buffer,
			const size_t count)
			-> std::shared_ptr<GpuBuffer>;

		static auto expandAndCopyBuffer(
			const std::shared_ptr<GpuLogicalDevice>& device,
			const std::shared_ptr<GpuBuffer>& buffer,
			const size_t count)
			->std::shared_ptr<GpuBuffer>;
		
		static void updateTexture(
			const std::shared_ptr<GpuLogicalDevice>& device,
			const std::shared_ptr<GpuCommandAllocator>& allocator,
			const std::shared_ptr<GpuCommandQueue>& queue,
			const std::shared_ptr<GpuTexture>& texture,
			const void* data
		);

		static auto loadTexture(
			const std::shared_ptr<GpuLogicalDevice>& device,
			const std::shared_ptr<GpuCommandAllocator>& allocator,
			const std::shared_ptr<GpuCommandQueue>& queue,
			const std::string& fileName
		) -> std::shared_ptr<GpuTexture>;
	private:
		static auto formatMapped(int channel) -> PixelFormat;
	};
	
}