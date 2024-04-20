#include "rendergraph_executor.h"

namespace HGEGraphics
{
	void Executor::Execute(CompiledRenderGraph& compiledRenderGraph, TexturePool& texturepool)
	{
		for (auto i = 0; i < compiledRenderGraph.passes.size(); ++i)
		{
			auto& pass = compiledRenderGraph.passes[i];

			for (auto resourceIndex : pass.devirtualize)
			{
				auto& resource = compiledRenderGraph.resources[resourceIndex];
				if (resource.type == RenderGraphResourceType::Managed)
				{
					resource.managered_texture = texturepool.getTexture(resource.width, resource.height, resource.format, resource.usage);
				}
			}

			//pass->record(m_renderSystem, subflow);

			for (auto resourceIndex : pass.destroy)
			{
				auto& resource = compiledRenderGraph.resources[resourceIndex];
				if (resource.type == RenderGraphResourceType::Managed)
					texturepool.releaseResource(resource.managered_texture);
			}
		}
	}
}