/*
** Haaf's Game Engine 1.8
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** Core functions implementation: graphics
*/


#include "hge_impl.h"
#include <d3d9.h>
#include <d3dx9.h>

void hge_graphicslog(void* user_data, ECGPULogSeverity severity, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	//vprintf(fmt, args);
	size_t nLen = _vscprintf(fmt, args) + 1;
	char* strBuffer = new char[nLen + 8];
	_vsnprintf_s(strBuffer, nLen, nLen, fmt, args);
	OutputDebugString(strBuffer);
	va_end(args);
}

size_t malloced = 0;
void *hge_graphicsmalloc(void *user_data, size_t size, const void *pool)
{
	if (size == 0)
		return nullptr;

	malloced += size;
	return malloc(size);
}

void* hge_graphicsrealloc(void* user_data, void* ptr, size_t size, const void* pool)
{
	malloced -= ptr ? _msize(ptr) : 0;
	malloced += size;
	return realloc(ptr, size);
}

void* hge_graphicscalloc(void* user_data, size_t count, size_t size, const void* pool)
{
	if (count * size == 0)
		return nullptr;

	malloced += count * size;
	return calloc(count, size);
}

void hge_graphicsfree(void *user_data, void *ptr, const void *pool)
{
	malloced -= ptr ? _msize(ptr) : 0;
	free(ptr);
}

size_t aligned_malloced = 0;
void *hge_graphicsmalloc_aligned(void *user_data, size_t size, size_t alignment, const void *pool)
{
	aligned_malloced += size;
	return _aligned_malloc(size, alignment);
}

void *hge_graphicsrealloc_aligned(void *user_data, void *ptr, size_t size, size_t alignment, const void *pool)
{
	aligned_malloced -= ptr ? _aligned_msize(ptr, alignment, 0) : 0;
	aligned_malloced += size;
	return _aligned_realloc(ptr, size, alignment);
}

void *hge_graphicscalloc_aligned(void *user_data, size_t count, size_t size, size_t alignment, const void *pool)
{
	aligned_malloced += count * size;
	void *memory = _aligned_malloc(count * size, alignment);
	if (memory != NULL)
		memset(memory, 0, count * size);
	return memory;
}

void hge_graphicsfree_aligned(void *user_data, void *ptr, size_t alignment, const void *pool)
{
	aligned_malloced -= ptr ? _aligned_msize(ptr, alignment, 0) : 0;
	_aligned_free(ptr);
}

void CALL HGE_Impl::Gfx_Clear(DWORD color)
{
	if (prepared)
		return;

	auto &cur_frame_data = frame_datas[current_frame_index];
	auto &cur_swapchain_info = swapchain_infos[current_swapchain_index];

	const CGPUClearValue clearColor = {
		.color = {GETR(color) / 255.0f, GETG(color) / 255.0f, GETB(color) / 255.0f, 1},
		.is_color = true,
	};

	CGPUBeginRenderPassInfo begin_info = {
		.render_pass = render_pass,
		.framebuffer = cur_swapchain_info.framebuffer,
		.clear_value_count = 1,
		.clear_values = &clearColor,
	};

	cur_rp_encoder = cgpu_cmd_begin_render_pass(cur_cmd, &begin_info);

	cgpu_render_encoder_set_viewport(cur_rp_encoder,
									 0.0f, 0.0f,
									 (float)nScreenWidth, (float)nScreenHeight,
									 0.f, 1.f);
	cgpu_render_encoder_set_scissor(cur_rp_encoder, 0, 0, nScreenWidth, nScreenHeight);

	prepared = true;
}

void CALL HGE_Impl::Gfx_SetClipping(int x, int y, int w, int h)
{ 

}

void CALL HGE_Impl::Gfx_SetTransform(float x, float y, float dx, float dy, float rot, float hscale, float vscale)
{

}

bool CALL HGE_Impl::Gfx_BeginScene(HTARGET targ)
{
	auto &cur_frame_data = frame_datas[current_frame_index];
	cgpu_wait_fences(&cur_frame_data.inflight_fence, 1);

	prepared = false;
	nPrim = 0;
	CurPrimType = 0;
	CurDefaultShaderPipeline = CGPU_NULLPTR;

	cgpu_reset_command_pool(cur_frame_data.pool);

	for (auto cmd : cur_frame_data.allocated_cmds)
		cur_frame_data.cmds.push_back(cmd);
	cur_frame_data.allocated_cmds.clear();

	CGPUAcquireNextDescriptor acquire_desc = {
		.signal_semaphore = cur_frame_data.prepared_semaphore,
	};

	current_swapchain_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
	auto &cur_swapchain_info = swapchain_infos[current_swapchain_index];

	auto back_buffer = cur_swapchain_info.texture;
	auto back_buffer_view = cur_swapchain_info.texture_view;
	auto prepared_semaphore = cur_frame_data.prepared_semaphore;

	cur_cmd = _RequestCmd(cur_frame_data);
	cgpu_cmd_begin(cur_cmd);

	CGPUTextureBarrier draw_barrier = {
		.texture = back_buffer,
		.src_state = CGPU_RESOURCE_STATE_UNDEFINED,
		.dst_state = CGPU_RESOURCE_STATE_RENDER_TARGET};
	CGPUResourceBarrierDescriptor barrier_desc0 = {.texture_barriers = &draw_barrier, .texture_barriers_count = 1};
	cgpu_cmd_resource_barrier(cur_cmd, &barrier_desc0);

	return true;
}

void CALL HGE_Impl::Gfx_EndScene()
{
	auto &cur_frame_data = frame_datas[current_frame_index];
	auto &cur_swapchain_info = swapchain_infos[current_swapchain_index];

	if (prepared)
	{
		_render_batch(true);

		cgpu_cmd_end_render_pass(cur_cmd, cur_rp_encoder);
	}

	auto back_buffer = cur_swapchain_info.texture;
	auto back_buffer_view = cur_swapchain_info.texture_view;
	auto prepared_semaphore = cur_frame_data.prepared_semaphore;

	CGPUTextureBarrier present_barrier = {
		.texture = back_buffer,
		.src_state = CGPU_RESOURCE_STATE_RENDER_TARGET,
		.dst_state = CGPU_RESOURCE_STATE_PRESENT};
	CGPUResourceBarrierDescriptor barrier_desc1 = {.texture_barriers = &present_barrier, .texture_barriers_count = 1};
	cgpu_cmd_resource_barrier(cur_cmd, &barrier_desc1);

	cgpu_cmd_end(cur_cmd);

	CGPUQueueSubmitDescriptor submit_desc = {
		.cmds = cur_frame_data.allocated_cmds.data(),
		.signal_fence = cur_frame_data.inflight_fence,
		.wait_semaphores = &prepared_semaphore,
		.signal_semaphores = &render_finished_semaphore,
		.cmds_count = (uint32_t)cur_frame_data.allocated_cmds.size(),
		.wait_semaphore_count = 1,
		.signal_semaphore_count = 1,
	};
	cgpu_submit_queue(gfx_queue, &submit_desc);

	CGPUQueuePresentDescriptor present_desc = {
		.swapchain = swapchain,
		.wait_semaphores = &render_finished_semaphore,
		.wait_semaphore_count = 1,
		.index = (uint8_t)current_swapchain_index,
	};
	cgpu_queue_present(present_queue, &present_desc);

	current_frame_index = (current_frame_index + 1) % swapchain->buffer_count;
}

void CALL HGE_Impl::Gfx_RenderLine(float x1, float y1, float x2, float y2, DWORD color, float z)
{
	if (VertArray)
	{
		if (CurPrimType != HGEPRIM_LINES || nPrim >= VERTEX_BUFFER_SIZE / HGEPRIM_LINES || CurTexture || CurBlendMode != BLEND_DEFAULT)
		{
			_render_batch();

			CurPrimType = HGEPRIM_LINES;
			if (CurBlendMode != BLEND_DEFAULT) _SetBlendMode(BLEND_DEFAULT);
			if (CurTexture)
			{
				//pD3DDevice->SetTexture(0, 0);
				CurTexture = 0;
			}
		}

		int i = nPrim * HGEPRIM_LINES;
		VertArray[i].x = x1; VertArray[i + 1].x = x2;
		VertArray[i].y = y1; VertArray[i + 1].y = y2;
		VertArray[i].z = VertArray[i + 1].z = z;
		VertArray[i].col = VertArray[i + 1].col = color;
		VertArray[i].tx = VertArray[i + 1].tx =
			VertArray[i].ty = VertArray[i + 1].ty = 0.0f;

		nPrim++;
	}
}

void CALL HGE_Impl::Gfx_RenderTriple(const hgeTriple *triple)
{
	if (VertArray)
	{
		if (CurPrimType != HGEPRIM_TRIPLES || nPrim >= VERTEX_BUFFER_SIZE / HGEPRIM_TRIPLES || CurTexture != triple->tex || CurBlendMode != triple->blend)
		{
			_render_batch();

			CurPrimType = HGEPRIM_TRIPLES;
			if (CurBlendMode != triple->blend) _SetBlendMode(triple->blend);
			if (triple->tex != CurTexture) {
				//pD3DDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)triple->tex);
				CurTexture = triple->tex;
			}
		}

		memcpy(&VertArray[nPrim * HGEPRIM_TRIPLES], triple->v, sizeof(hgeVertex) * HGEPRIM_TRIPLES);
		nPrim++;
	}
}

void CALL HGE_Impl::Gfx_RenderQuad(const hgeQuad *quad)
{
	if (VertArray)
	{
		if (CurPrimType != HGEPRIM_QUADS || nPrim >= VERTEX_BUFFER_SIZE / HGEPRIM_QUADS || CurTexture != quad->tex || CurBlendMode != quad->blend)
		{
			_render_batch();

			CurPrimType = HGEPRIM_QUADS;
			if (CurBlendMode != quad->blend) _SetBlendMode(quad->blend);
			if (quad->tex != CurTexture)
			{
				//pD3DDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)quad->tex);
				CurTexture = quad->tex;
			}
		}

		memcpy(&VertArray[nPrim * HGEPRIM_QUADS], quad->v, sizeof(hgeVertex) * HGEPRIM_QUADS);
		nPrim++;
	}
}

hgeVertex* CALL HGE_Impl::Gfx_StartBatch(int prim_type, HTEXTURE tex, int blend, int *max_prim)
{
	return 0;
}

void CALL HGE_Impl::Gfx_FinishBatch(int nprim)
{
	nPrim=nprim;
}

HTARGET CALL HGE_Impl::Target_Create(int width, int height, bool zbuffer)
{
	return 0;
}

void CALL HGE_Impl::Target_Free(HTARGET target)
{

}

HTEXTURE CALL HGE_Impl::Target_GetTexture(HTARGET target)
{
	return 0;
}

HTEXTURE CALL HGE_Impl::Texture_Create(int width, int height)
{
	return 0;
}

HTEXTURE CALL HGE_Impl::Texture_Load(const char *filename, DWORD size, bool bMipmap)
{
	return 0;
}

void CALL HGE_Impl::Texture_Free(HTEXTURE tex)
{

}

int CALL HGE_Impl::Texture_GetWidth(HTEXTURE tex, bool bOriginal)
{
	return 0;
}


int CALL HGE_Impl::Texture_GetHeight(HTEXTURE tex, bool bOriginal)
{
	return 0;
}


DWORD * CALL HGE_Impl::Texture_Lock(HTEXTURE tex, bool bReadOnly, int left, int top, int width, int height)
{
	return 0;
}


void CALL HGE_Impl::Texture_Unlock(HTEXTURE tex)
{

}

//////// Implementation ////////

void HGE_Impl::_render_batch(bool bEndScene)
{
	if (VertArray)
	{
		const uint32_t vert_stride = sizeof(hgeVertex);
		int eaten = 0;
		if (nPrim)
		{
			switch (CurPrimType)
			{
			case HGEPRIM_QUADS:
				eaten = nPrim << 2;
				break;

			case HGEPRIM_TRIPLES:
				eaten = nPrim * 3;
				break;

			case HGEPRIM_LINES:
				eaten = nPrim * 2;
				break;
			}

			if (eaten)
			{
				auto pipeline = _RequestPipeline(CurPrimType);
				if (pipeline != CurDefaultShaderPipeline)
				{
					cgpu_render_encoder_bind_pipeline(cur_rp_encoder, pipeline);
					CurDefaultShaderPipeline = pipeline;
				}
				cgpu_render_encoder_bind_vertex_buffers(cur_rp_encoder, 1, &pVB, &vert_stride, CGPU_NULLPTR);
				cgpu_render_encoder_draw(cur_rp_encoder, eaten, (VertArray - pVB->info->cpu_mapped_address) / vert_stride);
			}

			nPrim = 0;
		}

		if (bEndScene) VertArray = (hgeVertex*)pVB->info->cpu_mapped_address;
		else VertArray += eaten;
	}
}

void HGE_Impl::_SetBlendMode(int blend)
{
}

void HGE_Impl::_SetProjectionMatrix(int width, int height)
{
}

bool HGE_Impl::_GfxInit()
{
	CGPUInstanceDescriptor instance_desc = {
		.backend = CGPU_BACKEND_VULKAN,
		.enable_debug_layer = true,
		.enable_gpu_based_validation = true,
		.enable_set_name = true,
		.logger = {
			.log_callback = hge_graphicslog
		},
		.allocator = {
			.malloc_fn = hge_graphicsmalloc,
			.realloc_fn = hge_graphicsrealloc,
			.calloc_fn = hge_graphicscalloc,
			.free_fn = hge_graphicsfree,
			.malloc_aligned_fn = hge_graphicsmalloc_aligned,
			.realloc_aligned_fn = hge_graphicsrealloc_aligned,
			.calloc_aligned_fn = hge_graphicscalloc_aligned,
			.free_aligned_fn = hge_graphicsfree_aligned,
		},
	};
	instance = cgpu_create_instance(&instance_desc);

	uint32_t adapters_count = 0;
	cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
	CGPUAdapterId* adapters = (CGPUAdapterId*)_alloca(sizeof(CGPUAdapterId) * (adapters_count));
	cgpu_enum_adapters(instance, adapters, &adapters_count);
	auto adapter = adapters[0];

	CGPUQueueGroupDescriptor G = {
		.queue_type = CGPU_QUEUE_TYPE_GRAPHICS,
		.queue_count = 1
	};
	CGPUDeviceDescriptor device_desc = {
		.queue_groups = &G,
		.queue_group_count = 1
	};
	device = cgpu_create_device(adapter, &device_desc);
	gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
	present_queue = gfx_queue;

	surface = cgpu_surface_from_native_view(device, hwnd);

	ECGPUFormat swapchainFormat = CGPU_FORMAT_R8G8B8A8_SRGB;
	uint32_t swapchainCount = 1;
	CGPUSwapChainDescriptor descriptor = {
		.present_queues = &present_queue,
		.present_queues_count = 1,
		.surface = surface,
		.image_count = swapchainCount,
		.width = (uint32_t)nScreenWidth,
		.height = (uint32_t)nScreenHeight,
		.enable_vsync = true,
 		.format = swapchainFormat,
 	};
	swapchain = cgpu_create_swapchain(device, &descriptor);

	CGPUColorAttachment color_attachments = {
		.format = swapchainFormat,
		.load_action = CGPU_LOAD_ACTION_CLEAR,
		.store_action = CGPU_STORE_ACTION_STORE,
	};

 	CGPURenderPassDescriptor render_pass_descriptor = {
		.name = u8"Adore_Render_Pass",
		.sample_count = CGPU_SAMPLE_COUNT_1,
		.color_attachments = &color_attachments,
		.depth_stencil = nullptr,
		.render_target_count = 1,
	};
	render_pass = cgpu_create_render_pass(device, &render_pass_descriptor);

	swapchain_infos.clear();
	for (size_t i = 0; i < swapchain->buffer_count; ++i)
	{
		PerSwapChainInfo info;

		CGPUTextureViewDescriptor view_desc = {
			.texture = swapchain->back_buffers[i],
			.format = swapchain->back_buffers[i]->info->format,
			.usages = CGPU_TVU_RTV_DSV,
			.aspects = CGPU_TVA_COLOR,
			.dims = CGPU_TEX_DIMENSION_2D,
			.array_layer_count = 1,
		};
		info.texture = swapchain->back_buffers[i];
		info.texture_view = cgpu_create_texture_view(device, &view_desc);

		CGPUFramebufferDescriptor framebuffer_desc = {
			.renderpass = render_pass,
			.attachment_count = 1,
			.attachments = &info.texture_view,
			.width = (uint32_t)info.texture->info->width,
			.height = (uint32_t)info.texture->info->height,
			.layers = 1,
		};
		info.framebuffer = cgpu_create_framebuffer(device, &framebuffer_desc);

		swapchain_infos.push_back(info);
	}

	frame_datas.clear();
	for (size_t i = 0; i < swapchain->buffer_count; ++i)
	{
		PerFrameData frame_data;
		frame_data.inflight_fence = cgpu_create_fence(device);
		frame_data.prepared_semaphore = cgpu_create_semaphore(device);
		frame_data.pool = cgpu_create_command_pool(gfx_queue, CGPU_NULLPTR);
		frame_datas.push_back(frame_data);
	}

 	render_finished_semaphore = cgpu_create_semaphore(device);

	current_swapchain_index = 0;
	current_frame_index = 0;

	uint64_t vb_size = VERTEX_BUFFER_SIZE * sizeof(hgeVertex);
	CGPUBufferDescriptor vb_desc = {
		.size = vb_size,
		.name = u8"pVB",
		.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER,
		.memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
		.flags = CGPU_BCF_HOST_VISIBLE,
		.start_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	};
	pVB = cgpu_create_buffer(device, &vb_desc);

	CGPUBufferRange vb_range = { .offset = 0, .size = vb_size };
	cgpu_map_buffer(pVB, &vb_range);
	VertArray = (hgeVertex*)pVB->info->cpu_mapped_address;

	uint64_t ib_size = VERTEX_BUFFER_SIZE * 6 / 4 * sizeof(WORD);
	CGPUBufferDescriptor ib_desc = {
		.size = ib_size,
		.name = u8"pIB",
		.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER,
		.memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
		.flags = CGPU_BCF_HOST_VISIBLE,
		.start_state = CGPU_RESOURCE_STATE_INDEX_BUFFER,
	};
	pIB = cgpu_create_buffer(device, &ib_desc);

	if (!pIB)
	{
		_PostError("Can't lock D3D index buffer");
		return false;
	}

	CGPUBufferRange range = { .offset = 0, .size = ib_size };
	cgpu_map_buffer(pIB, &range);

	WORD* pIndices = (WORD*)pIB->info->cpu_mapped_address, n = 0;
	for (int i = 0; i < VERTEX_BUFFER_SIZE / 4; i++) {
		*pIndices++ = n;
		*pIndices++ = n + 1;
		*pIndices++ = n + 2;
		*pIndices++ = n + 2;
		*pIndices++ = n + 3;
		*pIndices++ = n;
		n += 4;
	}

	cgpu_unmap_buffer(pIB);

#include "hge.vert.spv.txt"
#include "hge.frag.spv.txt"

	CGPUShaderLibraryDescriptor vs_desc = {
		.name = u8"VertexShaderLibrary",
		.code = reinterpret_cast<const uint32_t*>(hge_vert_spv),
		.code_size = sizeof(hge_vert_spv),
		.stage = CGPU_SHADER_STAGE_VERT,
	};
	CGPUShaderLibraryDescriptor ps_desc = {
		.name = u8"FragmentShaderLibrary",
		.code = reinterpret_cast<const uint32_t*>(hge_frag_spv),
		.code_size = sizeof(hge_frag_spv),
		.stage = CGPU_SHADER_STAGE_FRAG,
	};
	CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
	CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
	default_shader[0].stage = CGPU_SHADER_STAGE_VERT;
	default_shader[0].entry = u8"main";
	default_shader[0].library = vertex_shader;
	default_shader[1].stage = CGPU_SHADER_STAGE_FRAG;
	default_shader[1].entry = u8"main";
	default_shader[1].library = fragment_shader;
	CGPURootSignatureDescriptor default_rs_desc = {
		.shaders = default_shader,
		.shader_count = 2
	};
	default_shader_root_sig = cgpu_create_root_signature(device, &default_rs_desc);

	return true;
}

void HGE_Impl::_AdjustWindow()
{
	RECT *rc;
	LONG style;

	if(bWindowed) {rc=&rectW; style=styleW; }
	else  {rc=&rectFS; style=styleFS; }
	SetWindowLong(hwnd, GWL_STYLE, style);

	style=GetWindowLong(hwnd, GWL_EXSTYLE);
	if(bWindowed)
	{
		SetWindowLong(hwnd, GWL_EXSTYLE, style & (~WS_EX_TOPMOST));
	    SetWindowPos(hwnd, HWND_NOTOPMOST, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_TOPMOST);
	    SetWindowPos(hwnd, HWND_TOPMOST, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, SWP_FRAMECHANGED);
	}
}

void HGE_Impl::_Resize(int width, int height)
{
	if(hwndParent)
	{

	}
}

void HGE_Impl::_GfxDone()
{
	cgpu_wait_queue_idle(gfx_queue);
	cgpu_wait_queue_idle(present_queue);

	cgpu_free_semaphore(render_finished_semaphore);
	render_finished_semaphore = CGPU_NULLPTR;

	for (size_t i = 0; i < frame_datas.size(); ++i)
	{
		auto frame_data = frame_datas[i];
		cgpu_free_fence(frame_data.inflight_fence);
		cgpu_free_semaphore(frame_data.prepared_semaphore);

		for (auto cmd : frame_data.cmds)
			cgpu_free_command_buffer(cmd);
		frame_data.cmds.clear();

		for (auto cmd : frame_data.allocated_cmds)
			cgpu_free_command_buffer(cmd);
		frame_data.allocated_cmds.clear();

		cgpu_free_command_pool(frame_data.pool);

		for (auto descriptor_set : frame_data.allocated_descriptor_sets)
			cgpu_free_descriptor_set(descriptor_set);
		frame_data.allocated_descriptor_sets.clear();
	}
	frame_datas.clear();

	for (size_t i = 0; i < swapchain_infos.size(); ++i)
	{
		auto info = swapchain_infos[i];
		cgpu_free_texture_view(info.texture_view);
		cgpu_free_framebuffer(info.framebuffer);
	}
	swapchain_infos.clear();

	if (pVB)
		cgpu_free_buffer(pVB);
	pVB = CGPU_NULLPTR;

	if (pIB)
		cgpu_free_buffer(pIB);
	pIB = CGPU_NULLPTR;

	for (auto [_, pipeline] : default_shader_pipelines)
	{
		cgpu_free_render_pipeline(pipeline);
	}
	default_shader_pipelines.clear();

	cgpu_free_root_signature(default_shader_root_sig);
	cgpu_free_shader_library(default_shader[0].library);
	cgpu_free_shader_library(default_shader[1].library);

	default_shader_root_sig = CGPU_NULLPTR;
	memset(default_shader, 0, 2 * sizeof(CGPUShaderEntryDescriptor));

	// for (auto shader : deleted_shaders)
	// 	deleteShaderImpl(shader);
	// deleted_shaders.clear();

	// for (auto image : deleted_images)
	// 	deleteImageImpl(image);
	// deleted_images.clear();

	cgpu_free_render_pass(render_pass);
	render_pass = CGPU_NULLPTR;

	cgpu_free_swapchain(swapchain);
	swapchain = CGPU_NULLPTR;
	cgpu_free_surface(device, surface);
	surface = CGPU_NULLPTR;

	cgpu_free_queue(gfx_queue);
	gfx_queue = CGPU_NULLPTR;
	present_queue = CGPU_NULLPTR;
	cgpu_free_device(device);
	device = CGPU_NULLPTR;
	cgpu_free_instance(instance);
	instance = CGPU_NULLPTR;
}

bool HGE_Impl::_GfxRestore()
{
	return true;
}


bool HGE_Impl::_init_lost()
{
	return true;
}

CGPUCommandBufferId HGE_Impl::_RequestCmd(PerFrameData &frame_data)
{
	CGPUCommandBufferId cmd;
	if (!frame_data.cmds.empty())
	{
		cmd = frame_data.cmds.back();
		frame_data.cmds.pop_back();
	}
	else
	{
		CGPUCommandBufferDescriptor cmd_desc = {.is_secondary = false};
		cmd = cgpu_create_command_buffer(frame_data.pool, &cmd_desc);
	}

	frame_data.allocated_cmds.push_back(cmd);
	return cmd;
}

CGPURenderPipelineId HGE_Impl::_RequestPipeline(int primType)
{
	uint32_t key = (0x3 & (primType - 1));

	auto iter = default_shader_pipelines.find(key);
	if (iter != default_shader_pipelines.end())
	{
		return iter->second;
	}
	else
	{
		ECGPUFormat swapchainFormat = CGPU_FORMAT_R8G8B8A8_SRGB;
		ECGPUFormat formats[1] = { swapchainFormat };
		CGPUVertexLayout vertex_layout = {
			.attribute_count = 3,
			.attributes = {
				{ u8"POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, CGPU_INPUT_RATE_VERTEX },
				{ u8"COLOR", 1, CGPU_FORMAT_R32_UINT, 0, sizeof(float) * 3, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX },
				{ u8"TEXCOORD0", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 3 + sizeof(uint32_t), sizeof(float) * 2, CGPU_INPUT_RATE_VERTEX },
			}
		};
		CGPUBlendStateDescriptor blend_state = {
			.src_factors = { CGPU_BLEND_CONST_ONE },
			.dst_factors = { CGPU_BLEND_CONST_ZERO },
			.src_alpha_factors = { CGPU_BLEND_CONST_ONE },
			.dst_alpha_factors = { CGPU_BLEND_CONST_ZERO },
			.blend_modes = { CGPU_BLEND_MODE_ADD },
			.blend_alpha_modes = { CGPU_BLEND_MODE_ADD },
			.masks = { CGPU_COLOR_MASK_ALL },
			.alpha_to_coverage = false,
			.independent_blend = false,
		};
		CGPUDepthStateDesc depth_state = {
			.depth_test = false,
			.depth_write = false,
			.stencil_test = false,
		};
		CGPURasterizerStateDescriptor rasterizer_state = {
			.cull_mode = CGPU_CULL_MODE_NONE,
		};
		ECGPUPrimitiveTopology prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
		if (primType == HGEPRIM_QUADS)
			prim_topology = CGPU_PRIM_TOPO_TRI_STRIP;
		else if (primType == HGEPRIM_TRIPLES)
			prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
		else if (primType == HGEPRIM_LINES)
			prim_topology = CGPU_PRIM_TOPO_LINE_LIST;
		CGPURenderPipelineDescriptor rp_desc = {
			.root_signature = default_shader_root_sig,
			.vertex_shader = &default_shader[0],
			.fragment_shader = &default_shader[1],
			.vertex_layout = &vertex_layout,
			.blend_state = &blend_state,
			.depth_state = &depth_state,
			.rasterizer_state = &rasterizer_state,
			.render_pass = render_pass,
			.subpass = 0,
			.render_target_count = 1,
			.prim_topology = prim_topology,
		};
		auto pipeline = cgpu_create_render_pipeline(device, &rp_desc);
		default_shader_pipelines.insert({ key, pipeline });

		return pipeline;
	}
}