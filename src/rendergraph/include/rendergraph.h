#pragma once

#include "cgpu/api.h"
#include <vector>
#include <array>
#include <memory>
#include <optional>
#include "texturepool.h"
#include <functional>

namespace HGEGraphics
{
	enum class RenderGraphResourceType
	{
		Managed,
		Imported,
		Backbuffer,
	};

	struct alignas(8) ColorAttachmentInfo
	{
		unsigned int clearColor = 0;
		int resourceIndex = 0;
		ECGPULoadAction load_action = ECGPULoadAction::CGPU_LOAD_ACTION_DONTCARE;
		ECGPUStoreAction store_action = ECGPUStoreAction::CGPU_STORE_ACTION_DISCARD;
		bool valid = false;
		bool operator==(const ColorAttachmentInfo& other) const;
	};

	struct alignas(8) DepthAttachmentInfo
	{
		float clearDepth = 0;
		uint8_t clearStencil = 0;
		int resourceIndex = 0;
		ECGPULoadAction depth_load_action = ECGPULoadAction::CGPU_LOAD_ACTION_DONTCARE;
		ECGPUStoreAction depth_store_action = ECGPUStoreAction::CGPU_STORE_ACTION_DISCARD;
		ECGPULoadAction stencil_load_action = ECGPULoadAction::CGPU_LOAD_ACTION_DONTCARE;
		ECGPUStoreAction stencil_store_action = ECGPUStoreAction::CGPU_STORE_ACTION_DISCARD;
		bool valid = false;
		bool operator==(const DepthAttachmentInfo& other) const;
	};

	class polymorphic_allocator_delete
	{
	public:
		polymorphic_allocator_delete(std::pmr::polymorphic_allocator<std::byte>& allocator)
			: d_allocator(allocator) {}
		template <typename T> void operator()(T* tPtr) 
		{
			std::pmr::polymorphic_allocator<T>(d_allocator).delete_object(tPtr);
		}

	private:
		std::pmr::polymorphic_allocator<std::byte>& d_allocator;
	};

	struct ResourceNode
	{
		ResourceNode(const char* name, uint16_t width, uint16_t height, ECGPUFormat format);
		ResourceNode(const char* name, CGPUTextureViewId texture);

		const char* name;
		const RenderGraphResourceType type;
		const CGPUTextureViewId texture;
		const uint16_t width;
		const uint16_t height;
		const ECGPUFormat format;
	};

	struct RenderGraphEdge
	{
		const uint16_t from;
		const uint16_t to;
		const TextureUsage usage;
	};

	typedef void(*RenderPassExecutable)(void* userdata);

	struct RenderPassNode
	{
		RenderPassNode(const char* name, std::pmr::memory_resource* const momory_resource);

		const char* name{ nullptr };
		std::pmr::vector<uint32_t> writes;
		std::pmr::vector<uint32_t> reads;
		int colorAttachmentCount{ 0 };
		std::array<ColorAttachmentInfo, 8> colorAttachments;
		DepthAttachmentInfo depthAttachment;
		RenderPassExecutable executable;
		void* userdata;
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

	struct RenderPassBuilder
	{
		RenderPassBuilder(class RenderGraph& renderGraph, RenderPassNode& passNode, int passIndex);

		static void addColorAttachment(RenderPassBuilder& passBuilder, RenderGraphHandle texture, ECGPULoadAction load_action, uint32_t clearColor, ECGPUStoreAction store_action);
		static void addDepthAttachment(RenderPassBuilder& passBuilder, RenderGraphHandle texture, ECGPULoadAction depth_load_action, float clearDepth, ECGPUStoreAction depth_store_action, ECGPULoadAction stencil_load_action, uint8_t clearStencil, ECGPUStoreAction stencil_store_action);
		static void sample(RenderPassBuilder& passBuilder, RenderGraphHandle texture);
		static void setExecutable(RenderPassBuilder& passBuilder, RenderPassExecutable executable, void* userdata);

	private:
		RenderGraph& renderGraph;
		RenderPassNode& passNode;
		uint16_t passIndex;
	};

	struct RenderGraph
	{
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

		RenderGraph(size_t estimate_resource_count, size_t estimate_pass_count, size_t estimate_edge_count, std::pmr::memory_resource* const resource);
		std::pmr::vector<ResourceNode> resources;
		std::pmr::vector<RenderPassNode> passes;
		std::pmr::vector<RenderGraphEdge> edges;
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
		static void reset(RenderGraph& renderGraph);
		static RenderGraphHandle declareTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, ECGPUFormat format);
		static RenderGraphHandle declareColorTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, ECGPUFormat format);
		static RenderGraphHandle declareDepthTexture(RenderGraph& renderGraph, const char* name, uint16_t width, uint16_t height, DepthBits depthBits, bool needStencil);
		static RenderGraphHandle importTexture(RenderGraph& renderGraph, const char* name, CGPUTextureViewId texture);
		static RenderPassBuilder addPass(RenderGraph& renderGraph, const char* name);
		static uint32_t addEdge(RenderGraph& renderGraph, uint32_t from, uint32_t to, TextureUsage usage);
	};
}