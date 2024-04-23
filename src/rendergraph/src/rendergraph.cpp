#include "rendergraph.h"

#include <cassert>

namespace HGEGraphics
{
	RenderGraph::RenderGraph(size_t estimate_resource_count, size_t estimate_pass_count, size_t estimate_edge_count, std::pmr::memory_resource* const resource)
		: allocator(resource), resources(resource), passes(resource), edges(resource)
	{
		resources.reserve(estimate_resource_count);
		passes.reserve(estimate_pass_count);
		edges.reserve(estimate_edge_count);
	}
	void Recorder::reset(RenderGraph& renderGraph)
	{
		renderGraph.resources.clear();
		renderGraph.passes.clear();
		renderGraph.edges.clear();
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
		renderGraph.resources.emplace_back(name, texture);
		return RenderGraphHandle(renderGraph.resources.size() - 1);
	}
	RenderPassBuilder Recorder::addPass(RenderGraph& renderGraph, const char* name)
	{
		renderGraph.passes.emplace_back(name, renderGraph.allocator.resource());
		return RenderPassBuilder(renderGraph, renderGraph.passes.back(), renderGraph.passes.size() - 1);
	}
	uint32_t Recorder::addEdge(RenderGraph& renderGraph, uint32_t from, uint32_t to, TextureUsage usage)
	{
		renderGraph.edges.emplace_back(from, to, usage);
		return renderGraph.edges.size() - 1;
	}
	ResourceNode::ResourceNode(const char* name, uint16_t width, uint16_t height, ECGPUFormat format)
		: name(name), type(RenderGraphResourceType::Managed), width(width), height(height), format (format), texture(CGPU_NULL)
	{
	}
	ResourceNode::ResourceNode(const char* name, CGPUTextureViewId texture)
		: name(name), type(RenderGraphResourceType::Backbuffer), texture(texture), width(texture->info.texture->info->width), height(texture->info.texture->info->height), format(texture->info.texture->info->format)
	{
	}
	RenderPassBuilder::RenderPassBuilder(RenderGraph& renderGraph, RenderPassNode& passNode, int passIndex)
		: renderGraph(renderGraph), passNode(passNode), passIndex(passIndex)
	{
	}
	RenderPassNode::RenderPassNode(const char* name, std::pmr::memory_resource* const momory_resource)
		: name(name), writes(momory_resource), reads(momory_resource)
	{
	}
	void RenderPassBuilder::addColorAttachment(RenderPassBuilder& passBuilder, RenderGraphHandle texture, ECGPULoadAction load_action, uint32_t clearColor, ECGPUStoreAction store_action)
	{
		assert(passBuilder.passNode.colorAttachmentCount <= passBuilder.passNode.colorAttachments.size());

		auto edge = Recorder::addEdge(passBuilder.renderGraph, passBuilder.passIndex, texture.index().value(), TextureUsage::ColorAttachment);
		passBuilder.passNode.writes.push_back(edge);
		passBuilder.passNode.colorAttachments[passBuilder.passNode.colorAttachmentCount++] =
		{
			.clearColor = clearColor,
			.resourceIndex = (int)passBuilder.passNode.writes.size() - 1,
			.load_action = load_action,
			.store_action = store_action,
			.valid = true,
		};
	}
	void RenderPassBuilder::addDepthAttachment(RenderPassBuilder& passBuilder, RenderGraphHandle texture, ECGPULoadAction depth_load_action, float clearDepth, ECGPUStoreAction depth_store_action, ECGPULoadAction stencil_load_action, uint8_t clearStencil, ECGPUStoreAction stencil_store_action)
	{
		assert(!passBuilder.passNode.depthAttachment.valid);

		auto edge = Recorder::addEdge(passBuilder.renderGraph, passBuilder.passIndex, texture.index().value(), TextureUsage::DepthAttachment);
		passBuilder.passNode.writes.push_back(edge);
		passBuilder.passNode.depthAttachment =
		{
			.clearDepth = clearDepth,
			.clearStencil = clearStencil,
			.resourceIndex = (int)passBuilder.passNode.writes.size() - 1,
			.depth_load_action = depth_load_action,
			.depth_store_action = depth_store_action,
			.stencil_load_action = stencil_load_action,
			.stencil_store_action = stencil_store_action,
			.valid = true,
		};
	}
	void RenderPassBuilder::sample(RenderPassBuilder& passBuilder, RenderGraphHandle texture)
	{
		auto edge = Recorder::addEdge(passBuilder.renderGraph, texture.index().value(), passBuilder.passIndex, TextureUsage::Sample);
		passBuilder.passNode.reads.push_back(edge);
	}
	void RenderPassBuilder::setExecutable(RenderPassBuilder& passBuilder, std::function<void()>&& executable)
	{
		passBuilder.passNode.executable = executable;
	}
}