#pragma once

#include "cgpu/api.h"
#include <vector>
#include <memory>
#include <optional>

namespace HGEGraphics
{
	enum class RenderGraphResourceType
	{
		Managed,
		Imported,
		Backbuffer,
	};

	enum class TextureUsage : uint16_t
	{
		None = 0b0000,
		Sample = 0x0004,
		ColorAttachment = 0x0010,
		DepthAttachment = 0x0020,
		InputAttachment = 0x0080,
		Present = 0x1000,
	};

	struct ResourceNode
	{
		ResourceNode(const char* name, uint16_t width, uint16_t height, ECGPUFormat format);
		ResourceNode(const char* name, CGPUTextureViewId texture, uint16_t width, uint16_t height, ECGPUFormat format);

		const RenderGraphResourceType type;
		const CGPUTextureViewId texture;
		const uint16_t width;
		const uint16_t height;
		const ECGPUFormat format;
	};

	struct PassNode
	{

	};

	struct RenderGraphEdge
	{

	};

	class RenderGraphHandle
	{
	public:
		RenderGraphHandle() :m_index(std::nullopt) {}
		std::optional<uint16_t> index() { return m_index; }

	private:
		friend struct Recorder;
		RenderGraphHandle(uint16_t index) : m_index(index) {}

	private:
		std::optional<uint16_t> m_index;
	};

	struct RenderGraph
	{
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

		RenderGraph(size_t estimate_resource_count, size_t estimate_pass_count, size_t estimate_edge_count, std::pmr::memory_resource* const resource);
		std::pmr::vector<std::unique_ptr<ResourceNode, std::pmr::polymorphic_allocator<ResourceNode>>> resources;
		std::pmr::vector<std::unique_ptr<PassNode, std::pmr::polymorphic_allocator<PassNode>>> passes;
		std::pmr::vector<std::unique_ptr<RenderGraphEdge, std::pmr::polymorphic_allocator<RenderGraphEdge>>> edges;
		allocator_type allocator;
	};

	enum class DepthBits : uint8_t
	{
		D32 = 0,
		D24 = 1,
		D16 = 2,
	};

	struct Recorder
	{
		static RenderGraphHandle declareTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, ECGPUFormat format);
		static RenderGraphHandle declareColorTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, ECGPUFormat format);
		static RenderGraphHandle declareDepthTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, DepthBits depthBits, bool needStencil);
		static RenderGraphHandle importTexture(RenderGraph& renderGraph, const char* name, CGPUTextureView texture);
		static int addPass(RenderGraph& renderGraph);
		static void present(RenderGraph& renderGraph, int texture);
	};

	class Compiler
	{

	};

	class Executor
	{

	};
}