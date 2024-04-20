#include "texturepool.h"

namespace HGEGraphics
{
	bool HGEGraphics::TextureDescriptor::operator==(const TextureDescriptor& other) const
	{
		return width == other.width && height == other.height
			&& depth == other.depth && mipLevels == other.mipLevels
			&& format == other.format && usage == other.usage;
	}

	TexturePool::TexturePool(TexturePool* upstream, std::pmr::memory_resource* const memory_resource)
		: ResourcePool(upstream, memory_resource)
	{
	}

	Texture* TexturePool::getTexture(uint16_t width, uint16_t height, ECGPUFormat format, TextureUsage usage)
	{
		TextureDescriptor key = { width, height, 1, 1, format, usage };
		return getResource(key);
	}
}

