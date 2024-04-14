#include "rendergraph.h"

namespace HGEGraphics
{
	RenderGraph::RenderGraph(size_t estimate_resource_count, size_t estimate_pass_count, size_t estimate_edge_count, std::pmr::memory_resource* const resource)
		: allocator(resource), resources(resource), passes(resource), edges(resource)
	{
		resources.reserve(estimate_resource_count);
		passes.reserve(estimate_pass_count);
		edges.reserve(estimate_edge_count);
	}
	RenderGraphHandle Recorder::declareTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, ECGPUFormat format)
	{
		renderGraph.resources.emplace_back(name, width, height, format);
		return RenderGraphHandle(renderGraph.resources.size() - 1);
	}
	RenderGraphHandle Recorder::declareColorTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, ECGPUFormat format)
	{
		return declareTexture(renderGraph, name, width, height, format);
	}
	RenderGraphHandle Recorder::declareDepthTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, DepthBits depthBits, bool needStencil)
	{
		auto FormatUtil_GetDepthStencilFormat = [] (DepthBits depthBits, bool needStencil) -> ECGPUFormat
			{
				if (depthBits == DepthBits::D32 && needStencil)
					return CGPU_FORMAT_D32_SFLOAT_S8_UINT;
				else if (depthBits == DepthBits::D32 && !needStencil)
					return CGPU_FORMAT_D32_SFLOAT;
				else if (depthBits == DepthBits::D24 && needStencil)
					return CGPU_FORMAT_D24_UNORM_S8_UINT;
				else if (depthBits == DepthBits::D24 && !needStencil)
					return CGPU_FORMAT_X8_D24_UNORM;
				else if (depthBits == DepthBits::D16 && needStencil)
					return CGPU_FORMAT_D16_UNORM_S8_UINT;
				else if (depthBits == DepthBits::D16 && !needStencil)
					return CGPU_FORMAT_D16_UNORM;
				else
					return CGPU_FORMAT_UNDEFINED;
			};

		auto format = FormatUtil_GetDepthStencilFormat(depthBits, needStencil);
		if (format != CGPU_FORMAT_UNDEFINED)
			return  declareTexture(renderGraph, name, width, height, format);
		else
			return RenderGraphHandle();
	}
	RenderGraphHandle Recorder::importTexture(RenderGraph& renderGraph, const char* name, CGPUTextureViewId texture)
	{
		renderGraph.resources.emplace_back(name, texture, (uint16_t)texture->info.texture->info->width, (uint16_t)texture->info.texture->info->height, texture->info.texture->info->format);
		return RenderGraphHandle(renderGraph.resources.size() - 1);
	}
	RenderPassBuilder Recorder::addPass(RenderGraph& renderGraph, const char* name)
	{
		renderGraph.passes.emplace_back(name, renderGraph.allocator);
		return RenderPassBuilder(renderGraph, renderGraph.passes.back());
	}
	ResourceNode::ResourceNode(const char* name, uint16_t width, uint16_t height, ECGPUFormat format)
		: type(RenderGraphResourceType::Managed), width(width), height(height), format (format), texture(CGPU_NULL)
	{
	}
	ResourceNode::ResourceNode(const char* name, CGPUTextureViewId texture, uint16_t width, uint16_t height, ECGPUFormat format)
		: type(RenderGraphResourceType::Managed), width(width), height(height), format(format), texture(texture)
	{
	}
	RenderPassBuilder::RenderPassBuilder(RenderGraph& renderGraph, RenderPassNode& passNode)
		: renderGraph(renderGraph), passNode(passNode)
	{
	}
	RenderPassNode::RenderPassNode(const char* name, std::pmr::polymorphic_allocator<std::byte>& allocator)
		: name(name), writes(allocator), reads(allocator)
	{
	}
	void RenderPassBuilder::addColorAttachment(RenderPassBuilder& passBuilder, RenderGraphHandle texture)
	{
		return ;
	}
}