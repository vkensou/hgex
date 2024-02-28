/*
** Haaf's Game Engine 1.8
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** Core functions implementation: graphics
*/


#include "hge_impl.h"
#include "FreeImage.h"
#include "glm/gtc/matrix_transform.hpp"
#include <span>

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
	_render_batch();

	if (vscale == 0.0f) matView = glm::identity<glm::mat4>();
	else
	{
		matView = glm::translate(glm::mat4(), glm::vec3(- x, -y, 0.0f));
		auto tmp = glm::scale(glm::mat4(), glm::vec3(hscale, vscale, 1.0f));
		matView = matView * tmp;
		tmp = glm::rotate(glm::mat4(), -rot, glm::vec3(0.0f, 0.0f, 1));
		matView = matView * tmp;
		tmp = glm::translate(glm::mat4(), glm::vec3(x + dx, y + dy, 0.0f));
		matView = matView * tmp;
	}

	// TODO
}

bool CALL HGE_Impl::Gfx_BeginScene(HTARGET targ)
{
	auto &cur_frame_data = frame_datas[current_frame_index];
	cgpu_wait_fences(&cur_frame_data.inflight_fence, 1);

	for (auto [texture, texture_view] : deleted_textures)
	{
		_DeleteDescriptorSet((HTEXTURE)texture_view);
		cgpu_free_texture_view(texture_view);
		cgpu_free_texture(texture);
	}
	deleted_textures.clear();

	prepared = false;
	nPrim = 0;
	CurPrimType = 0;
	CurDefaultShaderPipeline = CGPU_NULLPTR;
	CurDefaultDescriptorSet = CGPU_NULLPTR;
	CurBlendMode = 0;
	cur_vertex_buffer = vertexBuffers;
	VertArray = (hgeVertex*)cur_vertex_buffer->pVB->info->cpu_mapped_address;
	cur_vertex_buffer->ib_eaten = 0;
	cur_vertex_buffer->vb_eaten = 0;

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
	if (!prepared)
		Gfx_Clear(0);

	if (VertArray)
	{
		if (CurPrimType != HGEPRIM_LINES || nPrim >= VERTEX_BUFFER_SIZE / HGEPRIM_LINES || CurTexture || CurBlendMode != BLEND_DEFAULT)
		{
			_render_batch();

			CurPrimType = HGEPRIM_LINES;
			if (CurBlendMode != BLEND_DEFAULT) _SetBlendMode(BLEND_DEFAULT);
			if (CurTexture)
			{
				CurTexture = 0;
			}
		}

		int i = nPrim * HGEPRIM_LINES;
		hgeVertex line[2];
		line[0].x = x1; line[1].x = x2;
		line[0].y = y1; line[1].y = y2;
		line[0].z = line[1].z = z;
		line[0].col = line[1].col = color;
		line[0].tx = line[1].tx =
			line[0].ty = line[1].ty = 0.0f;

		_UploadVertexData(line);
	}
}

void CALL HGE_Impl::Gfx_RenderTriple(const hgeTriple *triple)
{
	if (!prepared)
		Gfx_Clear(0);

	if (VertArray)
	{
		if (CurPrimType != HGEPRIM_TRIPLES || nPrim >= VERTEX_BUFFER_SIZE / HGEPRIM_TRIPLES || CurTexture != triple->tex || CurBlendMode != triple->blend)
		{
			_render_batch();

			CurPrimType = HGEPRIM_TRIPLES;
			if (CurBlendMode != triple->blend) _SetBlendMode(triple->blend);
			if (triple->tex != CurTexture) {
				CurTexture = triple->tex;
			}
		}

		_UploadVertexData(triple->v);
	}
}

void CALL HGE_Impl::Gfx_RenderQuad(const hgeQuad *quad)
{
	if (!prepared)
		Gfx_Clear(0);

	if (VertArray)
	{
		if (CurPrimType != HGEPRIM_QUADS || nPrim >= VERTEX_BUFFER_SIZE / HGEPRIM_QUADS || CurTexture != quad->tex || CurBlendMode != quad->blend)
		{
			_render_batch();

			CurPrimType = HGEPRIM_QUADS;
			if (CurBlendMode != quad->blend) _SetBlendMode(quad->blend);
			if (quad->tex != CurTexture)
			{
				CurTexture = quad->tex;
			}
		}

		_UploadVertexData(quad->v);
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
	CGPUTextureDescriptor texture_desc =
	{
		.width = (uint64_t)width,
		.height = (uint64_t)height,
		.depth = 1,
		.array_size = 1,
		.format = CGPU_FORMAT_R8G8B8A8_UNORM,
		.mip_levels = 1,
		.owner_queue = gfx_queue,
		.start_state = CGPU_RESOURCE_STATE_COPY_DEST,
		.descriptors = CGPU_RESOURCE_TYPE_TEXTURE,
	};

	auto texture = cgpu_create_texture(device, &texture_desc);

	CGPUTextureViewDescriptor view_desc = {
		.texture = texture,
		.format = texture->info->format,
		.usages = CGPU_TVU_SRV,
		.aspects = CGPU_TVA_COLOR,
	};
	auto texture_view = cgpu_create_texture_view(device, &view_desc);

	auto texItem = new CTextureList;
	texItem->tex = texture;
	texItem->tex_view = texture_view;
	texItem->locked = NULL;
	texItem->next = textures;
	textures = texItem;

	return (HTEXTURE)texItem;
}

static void SwizzleFIBitmapDataToRGBA32(uint8_t* bits, int width, int height, int pitch)
{
	for (int y = 0; y < height; ++y)
	{
		uint32_t* pixel = (uint32_t*)bits;
		for (int x = 0; x < width; x++)
		{
			uint32_t c = pixel[x];
			pixel[x] = (((c & FI_RGBA_ALPHA_MASK) >> FI_RGBA_ALPHA_SHIFT) << 24)
				+ (((c & FI_RGBA_RED_MASK) >> FI_RGBA_RED_SHIFT) << 0)
				+ (((c & FI_RGBA_GREEN_MASK) >> FI_RGBA_GREEN_SHIFT) << 8)
				+ (((c & FI_RGBA_BLUE_MASK) >> FI_RGBA_BLUE_SHIFT) << 16);
		}
		bits += pitch;
	}
}

HTEXTURE CALL HGE_Impl::Texture_Load(const char *filename, DWORD size, bool bMipmap)
{
	void* data;
	DWORD _size;
	CTextureList* texItem;

	if (size) { data = (void*)filename; _size = size; }
	else
	{
		data = pHGE->Resource_Load(filename, &_size);
		if (!data) return NULL;
	}

	FIMEMORY* memory = FreeImage_OpenMemory((BYTE*)data, _size);
	if (!memory)
		return NULL;
	auto fiformat = FreeImage_GetFileTypeFromMemory(memory, _size);
	if (fiformat == FIF_UNKNOWN)
	{
		FreeImage_CloseMemory(memory);
		return NULL;
	}
	auto png = FreeImage_LoadFromMemory(fiformat, memory, PNG_DEFAULT);
	uint32_t bpp = FreeImage_GetBPP(png);
	if (bpp != 32)
	{
		auto prev_png = png;
		png = FreeImage_ConvertTo32Bits(prev_png);
		FreeImage_Unload(prev_png);
	}
	bpp = FreeImage_GetBPP(png);
	uint32_t width = FreeImage_GetWidth(png);
	uint32_t height = FreeImage_GetHeight(png);
	auto type = FreeImage_GetImageType(png);
	uint32_t pitch = FreeImage_GetPitch(png);
	auto bits = (uint8_t*)FreeImage_GetBits(png);
	SwizzleFIBitmapDataToRGBA32(bits, width, height, pitch);

	texItem = (CTextureList*)Texture_Create(width, height);

	auto locked = (uint8_t*)Texture_Lock((HTEXTURE)texItem);
	for (size_t i = 0; i < height; ++i)
	{
		memcpy((uint8_t*)locked + i * pitch, bits + i * pitch, pitch);
	}
	Texture_Unlock((HTEXTURE)texItem);

	FreeImage_CloseMemory(memory);
	if (!size) Resource_Free(data);

	return (HTEXTURE)texItem;
}

void CALL HGE_Impl::Texture_Free(HTEXTURE tex)
{
	CTextureList* pTextures = textures, * texPrev = 0;
	auto texItem = (CTextureList*)tex;

	while (pTextures)
	{
		if (pTextures == texItem)
		{
			if (texPrev) texPrev->next = pTextures->next;
			else textures = pTextures->next;
			deleted_textures.push_back(std::make_tuple(pTextures->tex, pTextures->tex_view));
			if (pTextures->locked) free(pTextures->locked);
			delete pTextures;
			break;
		}
		texPrev = pTextures;
		pTextures = pTextures->next;
	}
}

int CALL HGE_Impl::Texture_GetWidth(HTEXTURE tex, bool bOriginal)
{
	auto texItem = (CTextureList*)tex;
	if (texItem)
		return texItem->tex->info->width;
	else
		return 0;
}


int CALL HGE_Impl::Texture_GetHeight(HTEXTURE tex, bool bOriginal)
{
	auto texItem = (CTextureList*)tex;
	if (texItem)
		return texItem->tex->info->height;
	else
		return 0;
}


DWORD * CALL HGE_Impl::Texture_Lock(HTEXTURE tex, bool bReadOnly, int left, int top, int width, int height)
{
	auto texItem = (CTextureList*)tex;
	if (!texItem->locked)
	{
		texItem->locked = (uint8_t*)malloc(texItem->tex->info->width * texItem->tex->info->height * 4);
	}

	return (DWORD*)texItem->locked;
}

void CALL HGE_Impl::Texture_Unlock(HTEXTURE tex)
{
	auto texItem = (CTextureList*)tex;
	if (!texItem->locked)
		return;

	uint32_t width = texItem->tex->info->width;
	uint32_t height = texItem->tex->info->height;
	uint32_t bpp = 4;
	uint32_t pitch = width * bpp;
	CGPUCommandPoolDescriptor cmd_pool_desc = {};
	CGPUCommandBufferDescriptor cmd_desc = {};
	CGPUBufferDescriptor upload_buffer_desc = {};
	upload_buffer_desc.name = u8"UploadBuffer";
	upload_buffer_desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
	upload_buffer_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
	upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
	upload_buffer_desc.size = pitch * height;
	CGPUBufferId tex_upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
	{
		for (size_t i = 0; i < height; ++i)
		{
			memcpy((uint8_t*)tex_upload_buffer->info->cpu_mapped_address + (height - i - 1) * pitch, texItem->locked + i * pitch, pitch);
		}
	}
	auto cpy_cmd_pool = cgpu_create_command_pool(gfx_queue, &cmd_pool_desc);
	auto cpy_cmd = cgpu_create_command_buffer(cpy_cmd_pool, &cmd_desc);
	cgpu_cmd_begin(cpy_cmd);
	CGPUBufferToTextureTransfer b2t = {};
	b2t.src = tex_upload_buffer;
	b2t.src_offset = 0;
	b2t.dst = texItem->tex;
	b2t.dst_subresource.mip_level = 0;
	b2t.dst_subresource.base_array_layer = 0;
	b2t.dst_subresource.layer_count = 1;
	cgpu_cmd_transfer_buffer_to_texture(cpy_cmd, &b2t);
	CGPUTextureBarrier srv_barrier = {};
	srv_barrier.texture = texItem->tex;
	srv_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
	srv_barrier.dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
	CGPUResourceBarrierDescriptor barrier_desc1 = {};
	barrier_desc1.texture_barriers = &srv_barrier;
	barrier_desc1.texture_barriers_count = 1;
	cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc1);
	cgpu_cmd_end(cpy_cmd);
	CGPUQueueSubmitDescriptor cpy_submit = {};
	cpy_submit.cmds = &cpy_cmd;
	cpy_submit.cmds_count = 1;
	cgpu_submit_queue(gfx_queue, &cpy_submit);
	cgpu_wait_queue_idle(gfx_queue);
	cgpu_free_command_buffer(cpy_cmd);
	cgpu_free_command_pool(cpy_cmd_pool);
	cgpu_free_buffer(tex_upload_buffer);

	free(texItem->locked);
	texItem->locked = NULL;
}

//////// Implementation ////////

CVertexBufferList* createVertexBuffer(CGPUDeviceId device, uint32_t vertex_buffer_budget)
{
	uint64_t vb_size = vertex_buffer_budget * sizeof(hgeVertex);
	CGPUBufferDescriptor vb_desc = {
		.size = vb_size,
		.name = u8"pVB",
		.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER,
		.memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
		.flags = CGPU_BCF_HOST_VISIBLE,
		.start_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	};
	auto pVB = cgpu_create_buffer(device, &vb_desc);

	CGPUBufferRange vb_range = { .offset = 0, .size = vb_size };
	cgpu_map_buffer(pVB, &vb_range);

	uint64_t ib_size = vertex_buffer_budget * 6 / 4 * sizeof(WORD);
	CGPUBufferDescriptor ib_desc = {
		.size = ib_size,
		.name = u8"pIB",
		.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER,
		.memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
		.flags = CGPU_BCF_HOST_VISIBLE,
		.start_state = CGPU_RESOURCE_STATE_INDEX_BUFFER,
	};
	auto pIB = cgpu_create_buffer(device, &ib_desc);

	if (!pIB)
	{
		return false;
	}

	CGPUBufferRange ib_range = { .offset = 0, .size = ib_size };
	cgpu_map_buffer(pIB, &ib_range);

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

	auto vertexBuffer = new CVertexBufferList();
	vertexBuffer->pVB = pVB;
	vertexBuffer->pIB = pIB;
	vertexBuffer->vb_eaten = 0;
	vertexBuffer->ib_eaten = 0;
	vertexBuffer->next = NULL;
	return vertexBuffer;
}

void HGE_Impl::_render_batch(bool bEndScene)
{
	if (VertArray)
	{
		const uint32_t vert_stride = sizeof(hgeVertex);
		int vertex_eat = nPrim * CurPrimType, index_eat = nPrim * (CurPrimType == HGEPRIM_QUADS ? 6 : 0);
		if (nPrim)
		{
			if (vertex_eat)
			{
				bool blend = (CurBlendMode & BLEND_ALPHABLEND) != 0;
				bool color = (CurBlendMode & BLEND_COLORADD) != 0;

				auto pipeline = _RequestPipeline(CurPrimType, blend, color);
				if (pipeline != CurDefaultShaderPipeline)
				{
					cgpu_render_encoder_bind_pipeline(cur_rp_encoder, pipeline);
					CurDefaultShaderPipeline = pipeline;
				}
				auto descriptor_set = _RequestDescriptorSet(CurTexture, bTextureFilter, color);
				if (descriptor_set != CurDefaultDescriptorSet)
				{
					cgpu_render_encoder_bind_descriptor_set(cur_rp_encoder, descriptor_set);
					CurDefaultDescriptorSet = descriptor_set;
				}
				cgpu_render_encoder_bind_descriptor_set(cur_rp_encoder, per_frame_ubo_descriptor_sets[color]);
				cgpu_render_encoder_bind_vertex_buffers(cur_rp_encoder, 1, &cur_vertex_buffer->pVB, &vert_stride, CGPU_NULLPTR);
				cgpu_render_encoder_bind_index_buffer(cur_rp_encoder, cur_vertex_buffer->pIB, sizeof(WORD), 0);
				if (CurPrimType == HGEPRIM_QUADS)
					cgpu_render_encoder_draw_indexed(cur_rp_encoder, index_eat, cur_vertex_buffer->ib_eaten - index_eat, 0);
				else
					cgpu_render_encoder_draw(cur_rp_encoder, vertex_eat, cur_vertex_buffer->vb_eaten - vertex_eat);
			}

			nPrim = 0;
		}

		if (bEndScene) VertArray = 0;
		else VertArray += vertex_eat;
	}
}

void HGE_Impl::_SetBlendMode(int blend)
{
	CurBlendMode = blend;
}

void HGE_Impl::_SetProjectionMatrix(int width, int height)
{
	matProj = glm::orthoLH(0.0f, (float)width, (float)height, 0.0f, 0.0f, 1.0f);
	//matProj[1][1] *= -1;
}

CShader loadShader(CGPUDeviceId device, std::span<uint8_t> vs, std::span<uint8_t> ps)
{
	CGPUShaderLibraryDescriptor vs_desc = {
		.name = u8"VertexShaderLibrary",
		.code = reinterpret_cast<const uint32_t*>(vs.data()),
		.code_size = (uint32_t)vs.size(),
		.stage = CGPU_SHADER_STAGE_VERT,
	};
	CGPUShaderLibraryDescriptor ps_desc = {
		.name = u8"FragmentShaderLibrary",
		.code = reinterpret_cast<const uint32_t*>(ps.data()),
		.code_size = (uint32_t)ps.size(),
		.stage = CGPU_SHADER_STAGE_FRAG,
	};

	CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
	CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);

	CShader shader;
	shader.entry[0].stage = CGPU_SHADER_STAGE_VERT;
	shader.entry[0].entry = u8"main";
	shader.entry[0].library = vertex_shader;
	shader.entry[1].stage = CGPU_SHADER_STAGE_FRAG;
	shader.entry[1].entry = u8"main";
	shader.entry[1].library = fragment_shader;
	CGPURootSignatureDescriptor rs_desc = {
		.shaders = shader.entry,
		.shader_count = 2
	};
	shader.root_sigs = cgpu_create_root_signature(device, &rs_desc);

	return shader;
}

void freeShader(CShader shader)
{
	cgpu_free_root_signature(shader.root_sigs);
	cgpu_free_shader_library(shader.entry[0].library);
	cgpu_free_shader_library(shader.entry[1].library);
}

bool HGE_Impl::_GfxInit()
{
	FreeImage_Initialise(true);

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

	ECGPUFormat swapchainFormat = CGPU_FORMAT_R8G8B8A8_UNORM;
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

	vertexBuffers = createVertexBuffer(device, VERTEX_BUFFER_SIZE);
	cur_vertex_buffer = vertexBuffers;
	VertArray = (hgeVertex*)cur_vertex_buffer->pVB->info->cpu_mapped_address;

	uint8_t hge_vert_spv[] = {
#include "hge.vs.spv.h"
	};

	uint8_t hge_color_mul_frag_spv[] = {
#include "hge_color_mul.ps.spv.h"
	};

	uint8_t hge_color_add_frag_spv[] = {
#include "hge_color_add.ps.spv.h"
	};

	default_shaders[0] = loadShader(device, hge_vert_spv, hge_color_mul_frag_spv);
	default_shaders[1] = loadShader(device, hge_vert_spv, hge_color_add_frag_spv);

	CGPUSamplerDescriptor linear_sampler_desc = {
		.min_filter = CGPU_FILTER_TYPE_LINEAR,
		.mag_filter = CGPU_FILTER_TYPE_LINEAR,
		.mipmap_mode = CGPU_MIPMAP_MODE_NEAREST,
		.address_u = CGPU_ADDRESS_MODE_REPEAT,
		.address_v = CGPU_ADDRESS_MODE_REPEAT,
		.address_w = CGPU_ADDRESS_MODE_REPEAT,
		.mip_lod_bias = 0,
		.max_anisotropy = 1,
	};
	linear_sampler = cgpu_create_sampler(device, &linear_sampler_desc);

	CGPUSamplerDescriptor point_sampler_desc = {
		.min_filter = CGPU_FILTER_TYPE_NEAREST,
		.mag_filter = CGPU_FILTER_TYPE_NEAREST,
		.mipmap_mode = CGPU_MIPMAP_MODE_NEAREST,
		.address_u = CGPU_ADDRESS_MODE_REPEAT,
		.address_v = CGPU_ADDRESS_MODE_REPEAT,
		.address_w = CGPU_ADDRESS_MODE_REPEAT,
		.mip_lod_bias = 0,
		.max_anisotropy = 1,
	};
	point_sampler = cgpu_create_sampler(device, &point_sampler_desc);

	CGPUBufferDescriptor per_frame_ubo_desc = {
		.size = sizeof(PerFrameUBOData),
		.name = u8"pVB",
		.descriptors = CGPU_RESOURCE_TYPE_UNIFORM_BUFFER,
		.memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
		.flags = CGPU_BCF_HOST_VISIBLE,
		.start_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	};
	per_frame_ubo = cgpu_create_buffer(device, &per_frame_ubo_desc);

	CGPUDescriptorSetDescriptor per_frame_ubo_set_desc_0 = {
		.root_signature = default_shaders[0].root_sigs,
		.set_index = 1,
	};
	per_frame_ubo_descriptor_sets[0] = cgpu_create_descriptor_set(device, &per_frame_ubo_set_desc_0);

	CGPUDescriptorSetDescriptor per_frame_ubo_set_desc_1 = {
		.root_signature = default_shaders[1].root_sigs,
		.set_index = 1,
	};
	per_frame_ubo_descriptor_sets[1] = cgpu_create_descriptor_set(device, &per_frame_ubo_set_desc_1);

	_SetProjectionMatrix(nScreenWidth, nScreenHeight);
	matView = glm::mat4(1);

	CGPUBufferRange per_frame_ubo_range = { .offset = 0, .size = sizeof(PerFrameUBOData) };
	cgpu_map_buffer(per_frame_ubo, &per_frame_ubo_range);
	PerFrameUBOData ubo_data = {
		.matProj = matProj,
		.matView = matView,
	};
	*((PerFrameUBOData*)per_frame_ubo->info->cpu_mapped_address) = ubo_data;
	cgpu_unmap_buffer(per_frame_ubo);

	CGPUDescriptorData datas[1];
	datas[0] = {
		.binding = 0,
		.binding_type = CGPU_RESOURCE_TYPE_BUFFER,
		.buffers = &per_frame_ubo,
		.count = 1,
	};

	cgpu_update_descriptor_set(per_frame_ubo_descriptor_sets[0], datas, 1);
	cgpu_update_descriptor_set(per_frame_ubo_descriptor_sets[1], datas, 1);

	tex_white = Texture_Create(1, 1);
	auto pixels = Texture_Lock(tex_white);
	*pixels = RGBA(0xff, 0xff, 0xff, 0xff);
	Texture_Unlock(tex_white);

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
	while (textures)	
		Texture_Free((HTEXTURE)textures);
	tex_white = NULL;

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

	cgpu_free_sampler(linear_sampler);
	linear_sampler = CGPU_NULLPTR;

	cgpu_free_sampler(point_sampler);
	point_sampler = CGPU_NULLPTR;

	while (vertexBuffers)
	{
		if (vertexBuffers->pVB)
		{
			cgpu_unmap_buffer(vertexBuffers->pVB);
			cgpu_free_buffer(vertexBuffers->pVB);
		}

		if (vertexBuffers->pIB)
			cgpu_free_buffer(vertexBuffers->pIB);
		
		auto prev = vertexBuffers;
		vertexBuffers = vertexBuffers->next;
		delete prev;
	}


	for (auto [_, pipeline] : default_shader_pipelines)
	{
		cgpu_free_render_pipeline(pipeline);
	}
	default_shader_pipelines.clear();

	for (auto [_, descriptor_set] : default_shader_descriptor_sets)
	{
		cgpu_free_descriptor_set(descriptor_set);
	}

	freeShader(default_shaders[0]);
	freeShader(default_shaders[1]);
	memset(default_shaders, 0, sizeof(default_shaders));

	for (auto [texture, texture_view] : deleted_textures)
	{
		cgpu_free_texture_view(texture_view);
		cgpu_free_texture(texture);
	}
	deleted_textures.clear();

	cgpu_free_buffer(per_frame_ubo);
	per_frame_ubo = CGPU_NULLPTR;

	cgpu_free_descriptor_set(per_frame_ubo_descriptor_sets[0]);
	cgpu_free_descriptor_set(per_frame_ubo_descriptor_sets[1]);
	memset(per_frame_ubo_descriptor_sets, 0, sizeof(per_frame_ubo_descriptor_sets));

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

	FreeImage_DeInitialise();
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

CGPURenderPipelineId HGE_Impl::_RequestPipeline(int primType, bool blend, bool color)
{
	union
	{
		uint32_t value;
		struct {
			uint32_t primType : 2;
			uint32_t blend : 1;
			uint32_t color : 1;
			uint32_t pad : 28;
		} sep;
	} key;

	key.sep.primType = primType - 1;
	key.sep.blend = blend;
	key.sep.color = color;
	key.sep.pad = 0;

	auto iter = default_shader_pipelines.find(key.value);
	if (iter != default_shader_pipelines.end())
	{
		return iter->second;
	}
	else
	{
		ECGPUFormat swapchainFormat = CGPU_FORMAT_R8G8B8A8_UNORM;
		ECGPUFormat formats[1] = { swapchainFormat };
		CGPUVertexLayout vertex_layout = {
			.attribute_count = 3,
			.attributes = {
				{ u8"POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, CGPU_INPUT_RATE_VERTEX },
				{ u8"COLOR", 1, CGPU_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 3, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX },
				{ u8"TEXCOORD0", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 3 + sizeof(uint32_t), sizeof(float) * 2, CGPU_INPUT_RATE_VERTEX },
			}
		};
		CGPUBlendStateDescriptor blend_blend_state = {
			.src_factors = { CGPU_BLEND_CONST_SRC_ALPHA },
			.dst_factors = { CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA },
			.src_alpha_factors = { CGPU_BLEND_CONST_ONE },
			.dst_alpha_factors = { CGPU_BLEND_CONST_ZERO },
			.blend_modes = { CGPU_BLEND_MODE_ADD },
			.blend_alpha_modes = { CGPU_BLEND_MODE_ADD },
			.masks = { CGPU_COLOR_MASK_ALL },
			.alpha_to_coverage = false,
			.independent_blend = false,
		};
		CGPUBlendStateDescriptor blend_add_state = {
			.src_factors = { CGPU_BLEND_CONST_SRC_ALPHA },
			.dst_factors = { CGPU_BLEND_CONST_ONE },
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
			prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
		else if (primType == HGEPRIM_TRIPLES)
			prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
		else if (primType == HGEPRIM_LINES)
			prim_topology = CGPU_PRIM_TOPO_LINE_LIST;
		CGPURenderPipelineDescriptor rp_desc = {
			.root_signature = default_shaders[color].root_sigs,
			.vertex_shader = &default_shaders[color].entry[0],
			.fragment_shader = &default_shaders[color].entry[1],
			.vertex_layout = &vertex_layout,
			.blend_state = blend ? &blend_blend_state : &blend_add_state,
			.depth_state = &depth_state,
			.rasterizer_state = &rasterizer_state,
			.render_pass = render_pass,
			.subpass = 0,
			.render_target_count = 1,
			.prim_topology = prim_topology,
		};
		auto pipeline = cgpu_create_render_pipeline(device, &rp_desc);
		default_shader_pipelines.insert({ key.value, pipeline });

		return pipeline;
	}
}

CGPUDescriptorSetId HGE_Impl::_RequestDescriptorSet(HTEXTURE tex, bool linear, bool color)
{
	if (tex == NULL) {
		tex = tex_white; color = false;
	}
	auto texItem = (CTextureList*)tex;
	if (!texItem)
		return CGPU_NULLPTR;
	DescriptorSetKey key = { .tex = texItem, .sampler = linear, .color = color };
	auto iter = default_shader_descriptor_sets.find(key);
	if (iter != default_shader_descriptor_sets.end())
	{
		return iter->second;
	}
	else
	{
		CGPUDescriptorSetDescriptor set_desc = {
			.root_signature = default_shaders[color].root_sigs,
			.set_index = 0,
		};
		auto descriptor_set = cgpu_create_descriptor_set(device, &set_desc);

		CGPUTextureViewId texture_view = texItem->tex_view;
		CGPUSamplerId sampler = linear ? linear_sampler : point_sampler;

		CGPUDescriptorData datas[2];
		datas[0] = {
			.binding = 0,
			.binding_type = CGPU_RESOURCE_TYPE_TEXTURE,
			.textures = &texture_view,
			.count = 1,
		};
		datas[1] = {
			.binding = 1,
			.binding_type = CGPU_RESOURCE_TYPE_SAMPLER,
			.samplers = &sampler,
			.count = 1,
		};

		cgpu_update_descriptor_set(descriptor_set, datas, 2);
		default_shader_descriptor_sets.insert({ key, descriptor_set });

		return descriptor_set;
	}
}

void HGE_Impl::_DeleteDescriptorSet(HTEXTURE tex)
{
	auto texItem = (CTextureList*)tex;
	{
		DescriptorSetKey key = { .tex = texItem, .sampler = true };
		auto iter = default_shader_descriptor_sets.find(key);
		if (iter != default_shader_descriptor_sets.end())
		{
			cgpu_free_descriptor_set(iter->second);
			default_shader_descriptor_sets.erase(iter);
		}
	}

	{
		DescriptorSetKey key = { .tex = texItem, .sampler = false };
		auto iter = default_shader_descriptor_sets.find(key);
		if (iter != default_shader_descriptor_sets.end())
		{
			cgpu_free_descriptor_set(iter->second);
			default_shader_descriptor_sets.erase(iter);
		}
	}
}

bool HGE_Impl::_OutOfVertexBugets(uint32_t request_vertex_count, uint32_t request_index_count)
{
	return (cur_vertex_buffer->vb_eaten + request_vertex_count) > VERTEX_BUFFER_SIZE || (cur_vertex_buffer->ib_eaten + request_index_count) > VERTEX_BUFFER_SIZE * 6 / 4;
}

void HGE_Impl::_UploadVertexData(const hgeVertex* v)
{
	int vertex_eat = CurPrimType, index_eat = CurPrimType == HGEPRIM_QUADS ? 6 : 0;
	if (_OutOfVertexBugets(vertex_eat, index_eat))
	{
		_render_batch(false);

		if (!cur_vertex_buffer->next)
			cur_vertex_buffer->next = createVertexBuffer(device, VERTEX_BUFFER_SIZE);

		cur_vertex_buffer = cur_vertex_buffer->next;
		VertArray = (hgeVertex*)cur_vertex_buffer->pVB->info->cpu_mapped_address;
		cur_vertex_buffer->ib_eaten = 0;
		cur_vertex_buffer->vb_eaten = 0;
	}

	memcpy(&VertArray[nPrim * CurPrimType], v, sizeof(hgeVertex) * CurPrimType);
	nPrim++;
	cur_vertex_buffer->vb_eaten += vertex_eat;
	cur_vertex_buffer->ib_eaten += index_eat;
}
