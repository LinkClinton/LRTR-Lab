#include "ResourceHelper.hpp"

#include <stb_image.h>

void CodeRed::ResourceHelper::updateBuffer(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuCommandAllocator>& allocator,
	const std::shared_ptr<GpuBuffer>& buffer, 
	const void* data)
{
	updateBuffer(
		device,
		allocator,
		device->createCommandQueue(),
		buffer,
		data);
}

void CodeRed::ResourceHelper::updateBuffer(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuCommandAllocator>& allocator, 
	const std::shared_ptr<GpuCommandQueue>& queue,
	const std::shared_ptr<GpuBuffer>& buffer, 
	const void* data)
{
	//first, we create the command list and command queue for copy resource
	auto commandList = device->createGraphicsCommandList(allocator);
	auto commandQueue = queue;

	//create the upload buffer
	const auto uploadBuffer = device->createBuffer(
		ResourceInfo::UploadBuffer(buffer->stride(), buffer->count())
	);

	const auto memory = uploadBuffer->mapMemory();
	std::memcpy(memory, data, uploadBuffer->size());
	uploadBuffer->unmapMemory();

	const auto oldLayout = buffer->layout();

	//start recoding the copy commands
	commandList->beginRecording();

	//we need translate the buffer layout to copy-destination
	commandList->layoutTransition(buffer, ResourceLayout::CopyDestination);

	//copy upload buffer to buffer
	//because the size of constant buffer may not equal stride * count(256bytes limit)
	//so we use buffer size
	commandList->copyBuffer(uploadBuffer, buffer, buffer->size());

	//translate back the layout of buffer
	commandList->layoutTransition(buffer, oldLayout);

	//end recoding, execute and wait finish
	commandList->endRecording();
	commandQueue->execute({ commandList });
	commandQueue->waitIdle();
}

void CodeRed::ResourceHelper::updateBuffer(
	const std::shared_ptr<GpuBuffer>& buffer, 
	const void* data, 
	const size_t size)
{
	if (size == 0) return;
	
	const auto memory = buffer->mapMemory();

	std::memcpy(memory, data, size);

	buffer->unmapMemory();
}

void CodeRed::ResourceHelper::updateBuffer(
	const std::shared_ptr<GpuBuffer>& buffer, 
	const void* data,
	const size_t offset, 
	const size_t size)
{
	if (size == 0) return;
	
	const auto memory = 
		reinterpret_cast<void*>(
		reinterpret_cast<size_t>(buffer->mapMemory()) + offset);
	
	std::memcpy(memory, data, size);

	buffer->unmapMemory();
}

void CodeRed::ResourceHelper::copyBuffer(
	const std::shared_ptr<GpuBuffer>& destination,
	const std::shared_ptr<GpuBuffer>& source, 
	const size_t offset)
{
	const auto dstMemory =
		reinterpret_cast<void*>(
			reinterpret_cast<size_t>(destination->mapMemory()) + offset);
	const auto srcMemory = source->mapMemory();

	std::memcpy(dstMemory, srcMemory, source->size());

	destination->unmapMemory();
	source->unmapMemory();
}

auto CodeRed::ResourceHelper::expandBuffer(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuBuffer>& buffer,
	const size_t count)
	-> std::shared_ptr<GpuBuffer>
{
	if (buffer->count() >= count) return buffer;

	auto bufferCount = buffer->count();

	while (bufferCount < count) bufferCount <<= 1;
	
	auto newBuffer = device->createBuffer(
		ResourceInfo(
			BufferProperty(buffer->stride(), bufferCount),
			buffer->layout(),
			buffer->usage(),
			buffer->type(),
			buffer->heap())
	);

	return newBuffer;
}

auto CodeRed::ResourceHelper::expandAndCopyBuffer(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuBuffer>& buffer,
	const size_t count)
	-> std::shared_ptr<GpuBuffer>
{
	if (buffer->count() >= count) return buffer;

	auto bufferCount = buffer->count();

	while (bufferCount < count) bufferCount <<= 1;

	auto newBuffer = device->createBuffer(
		ResourceInfo(
			BufferProperty(buffer->stride(), bufferCount),
			buffer->layout(),
			buffer->usage(),
			buffer->type(),
			buffer->heap())
	);

	copyBuffer(newBuffer, buffer, 0);

	return newBuffer;
}

void CodeRed::ResourceHelper::updateTexture(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuCommandAllocator>& allocator,
	const std::shared_ptr<GpuCommandQueue>& queue,
	const std::shared_ptr<GpuTexture>& texture,
	const void* data)
{
	auto commandList = device->createGraphicsCommandList(allocator);
	auto commandQueue = queue;

	auto oldLayout = texture->layout();

	CODE_RED_TRY_EXECUTE(
		oldLayout == ResourceLayout::Undefined,
		oldLayout = ResourceLayout::GeneralRead
	);
	
	commandList->beginRecording();

	commandList->layoutTransition(texture, ResourceLayout::CopyDestination);

	commandList->copyMemoryToTexture(texture, data);

	commandList->layoutTransition(texture, oldLayout);

	commandList->endRecording();
	commandQueue->execute({ commandList });
	commandQueue->waitIdle();
}

auto CodeRed::ResourceHelper::loadTexture(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuCommandAllocator>& allocator, 
	const std::shared_ptr<GpuCommandQueue>& queue,
	const std::string& fileName)
	-> std::shared_ptr<GpuTexture>
{
	auto width = 0;
	auto height = 0;
	auto channel = 0;
	
	const auto data = stbi_load(fileName.c_str(), &width, &height, &channel, STBI_rgb_alpha);

	auto texture = device->createTexture(
		ResourceInfo::Texture2D(
			width,
			height,
			PixelFormat::RedGreenBlueAlpha8BitUnknown
		)
	);

	updateTexture(device, allocator, queue, texture, data);

	stbi_image_free(data);
	
	return texture;
}

auto CodeRed::ResourceHelper::formatMapped(int channel) -> PixelFormat
{
	//we only enable the channel 1 or 4
	CODE_RED_DEBUG_THROW_IF(
		channel != 1 && channel != 4,
		Exception("the format of texture is not support.")
	);

	return channel == 1 ? PixelFormat::Red8BitUnknown : PixelFormat::RedGreenBlueAlpha8BitUnknown;
}
