#pragma once

#include "rendergraph_compiler.h"
#include "texturepool.h"

namespace HGEGraphics
{
	struct Executor
	{
		static void Execute(CompiledRenderGraph& compiledRenderGraph, TexturePool& texturepool);
	};
}