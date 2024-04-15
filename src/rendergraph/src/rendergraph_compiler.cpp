#include "rendergraph_compiler.h"

#include "dependencygraph.h"

namespace HGEGraphics
{
	CompiledRenderGraph Compiler::Compile(const RenderGraph& renderGraph, std::pmr::memory_resource* const resource)
	{
		auto resourceCount = renderGraph.resources.size();
		auto passCount = renderGraph.passes.size();
		auto edgeCount = renderGraph.edges.size();

		HGEGraphics::DependencyGraph graph(resourceCount + passCount, edgeCount, resource);
		









		return CompiledRenderGraph();
	}
}