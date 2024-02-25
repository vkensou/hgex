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
	vprintf(fmt, args);
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

}

void CALL HGE_Impl::Gfx_SetClipping(int x, int y, int w, int h)
{

}

void CALL HGE_Impl::Gfx_SetTransform(float x, float y, float dx, float dy, float rot, float hscale, float vscale)
{

}

bool CALL HGE_Impl::Gfx_BeginScene(HTARGET targ)
{

	return true;
}

void CALL HGE_Impl::Gfx_EndScene()
{

}

void CALL HGE_Impl::Gfx_RenderLine(float x1, float y1, float x2, float y2, DWORD color, float z)
{

}

void CALL HGE_Impl::Gfx_RenderTriple(const hgeTriple *triple)
{

}

void CALL HGE_Impl::Gfx_RenderQuad(const hgeQuad *quad)
{

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
			.width = (uint32_t)nScreenWidth,
			.height = (uint32_t)nScreenHeight,
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