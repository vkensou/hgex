#include "rendergraph_compiler.h"

#include "dependencygraph.h"
#include <bitset>

namespace HGEGraphics
{
	CompiledRenderGraph Compiler::Compile(const RenderGraph& renderGraph, std::pmr::memory_resource* const resource)
	{
		auto resourceCount = renderGraph.resources.size();
		auto passCount = renderGraph.passes.size();
		auto edgeCount = renderGraph.edges.size();

		HGEGraphics::DependencyGraph graph(resourceCount + passCount, edgeCount, resource);
		
		struct Node
		{
			Node(std::pmr::memory_resource* const resource)
				: reads(resource), writes(resource)
			{
			}

			std::pmr::vector<uint16_t> reads;
			std::pmr::vector<uint16_t> writes;
			uint16_t index = 0;
			bool is_root = false;
		};

		std::pmr::vector<Node> nodes(resource);
		nodes.reserve(renderGraph.passes.size());
		for (uint16_t i = 0; i < renderGraph.passes.size(); ++i)
		{
			auto const& pass = renderGraph.passes[i];
			Node node(resource);
			node.index = i;
			node.is_root = pass.is_root;
			node.reads.reserve(pass.reads.size());
			for (uint16_t j = 0; j < pass.reads.size(); ++j)
				node.reads.push_back(j);
			node.writes.reserve(pass.writes.size());
			for (uint16_t k = 0; k < pass.writes.size(); ++k)
				node.writes.push_back(k);
			nodes.emplace_back(std::move(node));
		}






		return CompiledRenderGraph();
	}
}