#pragma once

#include "resourcepool.h"
#include "cgpu/api.h"
#include "hash.h"

namespace HGEGraphics
{
	enum class TextureUsage : uint16_t
	{
		None = 0b0000,
		Sample = 0x0004,
		ColorAttachment = 0x0010,
		DepthAttachment = 0x0020,
		InputAttachment = 0x0080,
		Present = 0x1000,
	};

	struct TextureDescriptor
	{
		uint16_t width = 0;
		uint16_t height = 0;
		uint16_t depth = 0;
		uint16_t mipLevels = 0;
		ECGPUFormat format = CGPU_FORMAT_UNDEFINED;
		TextureUsage usage = TextureUsage::None;

		bool operator==(const TextureDescriptor& other) const;
	};
}

namespace std
{
	template <>
	struct hash<HGEGraphics::TextureDescriptor> {
		size_t operator()(const HGEGraphics::TextureDescriptor& xyz) const {
			return HGEGraphics::MurmurHashFn<HGEGraphics::TextureDescriptor>()(xyz);
		}
	};
}

namespace HGEGraphics
{
	struct Texture
	{
		CGPUTextureId texture;
		CGPUTextureViewId texture_view;
	};

	class TexturePool
		: public ResourcePool<TextureDescriptor, Texture, false>
	{
	public:
		TexturePool(TexturePool* upstream, std::pmr::memory_resource* const memory_resource);

		Texture* getTexture(uint16_t width, uint16_t height, ECGPUFormat format, TextureUsage usage = TextureUsage::None);
	};
}
