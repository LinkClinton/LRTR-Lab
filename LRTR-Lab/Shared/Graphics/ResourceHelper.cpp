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

	size_t offset = 0;

	auto bufferPool = std::vector<std::shared_ptr<GpuTextureBuffer>>();

	auto oldLayout = texture->layout();
	
	commandList->beginRecording();
	commandList->layoutTransition(texture, ResourceLayout::CopyDestination);
	
	for (size_t arraySlice = 0; arraySlice < texture->arrays(); arraySlice++) {
		for (size_t mipSlice = 0; mipSlice < texture->mipLevels(); mipSlice++) {
			const auto buffer = device->createTextureBuffer(texture, mipSlice);

			buffer->write(static_cast<const unsigned char*>(data) + offset);

			offset = offset + buffer->size();

			commandList->layoutTransition(buffer, ResourceLayout::CopySource);
			commandList->copyBufferToTexture(
				TextureBufferCopyInfo(buffer),
				TextureCopyInfo(texture, texture->index(mipSlice, arraySlice)),
				buffer->width(), buffer->height(), buffer->depth());

			bufferPool.push_back(buffer);
		}
	}
	
	commandList->layoutTransition(texture, oldLayout);
	commandList->endRecording();

	commandQueue->execute({ commandList });
	commandQueue->waitIdle();
}

auto CodeRed::ResourceHelper::readTexture(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuCommandAllocator>& allocator, 
	const std::shared_ptr<GpuCommandQueue>& queue,
	const std::shared_ptr<GpuTexture>& texture)
	-> std::vector<Byte>
{
	auto commandList = device->createGraphicsCommandList(allocator);
	auto commandQueue = queue;

	auto bufferPool = std::vector<std::shared_ptr<GpuTextureBuffer>>();

	auto oldLayout = texture->layout();

	commandList->beginRecording();
	commandList->layoutTransition(texture, ResourceLayout::CopySource);
	
	for (size_t arraySlice = 0; arraySlice < texture->arrays(); arraySlice++) {
		for (size_t mipSlice = 0; mipSlice < texture->mipLevels(); mipSlice++) {
			const auto buffer = device->createTextureBuffer(texture, mipSlice);

			commandList->layoutTransition(buffer, ResourceLayout::CopyDestination);
			commandList->copyTextureToBuffer(
				TextureCopyInfo(texture, texture->index(mipSlice, arraySlice)),
				TextureBufferCopyInfo(buffer), 
				buffer->width(), buffer->height(), buffer->depth());

			bufferPool.push_back(buffer);
		}
	}

	commandList->layoutTransition(texture, oldLayout);
	commandList->endRecording();

	commandQueue->execute({ commandList });
	commandQueue->waitIdle();

	auto result = std::vector<Byte>();
	
	for (const auto& buffer : bufferPool) {
		const auto data = buffer->read();

		result.insert(result.end(), data.begin(), data.end());
	}

	return result;
}

auto CodeRed::ResourceHelper::loadTexture(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuCommandAllocator>& allocator, 
	const std::shared_ptr<GpuCommandQueue>& queue,
	const std::string& fileName,
	const PixelFormat format)
	-> std::shared_ptr<GpuTexture>
{
	auto width = 0;
	auto height = 0;
	auto channel = 0;

	void* data = nullptr;

	if (PixelFormatSizeOf::get(format) == 4)
		data = stbi_load(fileName.c_str(), &width, &height, &channel, STBI_rgb_alpha);
	if (PixelFormatSizeOf::get(format) == 16)
		data = stbi_loadf(fileName.c_str(), &width, &height, &channel, STBI_rgb_alpha);
	
	auto texture = device->createTexture(
		ResourceInfo::Texture2D(
			width,
			height,
			format
		)
	);

	updateTexture(device, allocator, queue, texture, data);

	stbi_image_free(data);
	
	return texture;
}

auto CodeRed::ResourceHelper::loadSkyBox(
	const std::shared_ptr<GpuLogicalDevice>& device,
	const std::shared_ptr<GpuCommandAllocator>& allocator, 
	const std::shared_ptr<GpuCommandQueue>& queue,
	const std::string& dirName) -> std::shared_ptr<GpuTexture>
{
	const std::string fileName[6] = {
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"front.jpg",
		"back.jpg"
	};

	stbi_uc* skyBoxImages[6];
	
	auto width = 0;
	auto height = 0;
	auto channel = 0;
	
	for (size_t index = 0; index < 6; index++) {
		skyBoxImages[index] = stbi_load((dirName + "/" + fileName[index]).c_str(), &width, &height, &channel,
			STBI_rgb_alpha);
	}

	const auto skyBoxData = new unsigned char[height * width * 24];
	const auto depthPitch = width * height * 4;

	for (size_t index = 0; index < 6; index++) {
		std::memcpy(skyBoxData + index * depthPitch, skyBoxImages[index], depthPitch);
		
		stbi_image_free(skyBoxImages[index]);
	}

	const auto texture = device->createTexture(
		ResourceInfo::CubeMap(
			width,
			height,
			PixelFormat::RedGreenBlueAlpha8BitUnknown
		)
	);

	updateTexture(device, allocator, queue, texture, skyBoxData);

	delete[] skyBoxData;

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
