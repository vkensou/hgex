#include "rendergraph_compiler.h"

#include "dependencygraph.h"
#include <cassert>

namespace HGEGraphics
{
	CompiledRenderGraph Compiler::Compile(const RenderGraph& renderGraph, std::pmr::memory_resource* const memory_resource)
	{
		auto resourceCount = renderGraph.resources.size();
		auto passCount = renderGraph.passes.size();
		auto edgeCount = renderGraph.edges.size();

		HGEGraphics::DependencyGraph graph(resourceCount + passCount, edgeCount, memory_resource);
		
		struct Node
		{
			Node(uint16_t index, bool is_pass, bool is_persistent, std::pmr::memory_resource* const resource)
				: index(index), is_pass(is_pass), is_persistent(is_persistent), ins(resource), outs(resource)
			{
			}

			bool is_culled() const
			{
				return ref_count == 0 && !is_persistent;
			}

			std::pmr::vector<uint16_t> ins;
			std::pmr::vector<uint16_t> outs;
			uint16_t index = 0;
			uint16_t ref_count = 0;
			bool is_pass;
			bool is_persistent{ false };
		};

		std::pmr::vector<Node> nodes(memory_resource);
		nodes.reserve(passCount + resourceCount);
		for (uint16_t i = 0; i < passCount; ++i)
		{
			auto const& pass = renderGraph.passes[i];
			Node node(i, true, false, memory_resource);
			node.ins.reserve(pass.reads.size());
			for (uint16_t j = 0; j < pass.reads.size(); ++j)
				node.ins.push_back(renderGraph.edges[pass.reads[j]].from + passCount);
			node.outs.reserve(pass.writes.size());
			for (uint16_t j = 0; j < pass.writes.size(); ++j)
				node.outs.push_back(renderGraph.edges[pass.writes[j]].to + passCount);
			nodes.emplace_back(std::move(node));
		}

		for (uint16_t i = 0; i < resourceCount; ++i)
		{
			auto const& resource = renderGraph.resources[i];
			Node node(i, false, resource.is_imported, memory_resource);
			nodes.emplace_back(std::move(node));
		}

		for (uint16_t i = 0; i < passCount; ++i)
		{
			auto const& node = nodes[i];
			for (uint16_t j = 0; j < node.ins.size(); ++j)
			{
				auto the_in = node.ins[j];
				nodes[the_in].outs.push_back(i);
			}
			for (uint16_t j = 0; j < node.outs.size(); ++j)
			{
				auto the_out = node.outs[j];
				nodes[the_out].ins.push_back(i);
			}
		}

		for (auto i = 0; i < nodes.size(); ++i)
		{
			auto& node = nodes[i];
			node.ref_count = node.outs.size();
		}

		std::pmr::vector<uint16_t> cullingStack(memory_resource);
		cullingStack.reserve(nodes.size());
		for (auto i = 0; i < nodes.size(); ++i)
		{
			auto& node = nodes[i];
			if (node.ref_count == 0 && !node.is_persistent)
				cullingStack.push_back(i);
		}

		while (!cullingStack.empty())
		{
			auto index = cullingStack.back();
			cullingStack.pop_back();
			auto const& node = nodes[index];

			for (auto i = 0; i < node.ins.size(); ++i)
			{
				auto inNodeIndex = node.ins[i];
				auto& inNode = nodes[inNodeIndex];
				assert(inNode.ref_count > 0);
				inNode.ref_count--;
				if (inNode.ref_count == 0 && !inNode.is_persistent)
					cullingStack.push_back(inNodeIndex);
			}
		}

		return CompiledRenderGraph();
	}
}