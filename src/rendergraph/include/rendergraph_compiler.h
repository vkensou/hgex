#pragma once

#include "rendergraph.h"

namespace HGEGraphics
{
	struct CompiledResourceNode
	{
		CompiledResourceNode(const char* name, RenderGraphResourceType type, uint16_t width, uint16_t height, ECGPUFormat format, CGPUTextureViewId imported_texture_view);

		const char* name;
		const RenderGraphResourceType type;
		Texture* managered_texture;
		CGPUTextureViewId imported_texture_view;
		const uint16_t width;
		const uint16_t height;
		const ECGPUFormat format;
		TextureUsage usage;
	};

	struct CompiledRenderPassNode
	{
		CompiledRenderPassNode(const char* name, std::pmr::memory_resource* const memory_resource);

		const char* name{ nullptr };
		std::pmr::vector<uint16_t> writes;
		std::pmr::vector<uint16_t> reads;
		std::pmr::vector<uint16_t> devirtualize;
		std::pmr::vector<uint16_t> destroy;
		int colorAttachmentCount{ 0 };
		std::array<ColorAttachmentInfo, 8> colorAttachments;
		DepthAttachmentInfo depthAttachment;
	};

	struct CompiledRenderGraph
	{
		CompiledRenderGraph(std::pmr::memory_resource* const memory_resource);
		std::pmr::vector<CompiledResourceNode> resources;
		std::pmr::vector<CompiledRenderPassNode> passes;
	};

	struct Compiler
	{
		static CompiledRenderGraph Compile(const RenderGraph& renderGraph, std::pmr::memory_resource* const memory_resource);
	};
}