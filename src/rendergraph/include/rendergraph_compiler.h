#pragma once

#include "rendergraph.h"

namespace HGEGraphics
{
	struct CompiledRenderGraph
	{

	};

	struct Compiler
	{
		static CompiledRenderGraph Compile(const RenderGraph& renderGraph, std::pmr::memory_resource* const resource);
	};
}