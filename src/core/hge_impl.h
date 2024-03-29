/*
** Haaf's Game Engine 1.8
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** Common core implementation header
*/


#ifndef HGE_IMPL_H
#define HGE_IMPL_H

#include "..\..\include\hge.h"
#include <stdio.h>
#include "cgpu/api.h"
#include <vector>
#include <unordered_map>
#include "glm/mat4x4.hpp"
#include "renderdoc.h"
#include "utils/JobSystem.h"

#define DEMO

#define D3DFVF_HGEVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define VERTEX_BUFFER_SIZE 4000


typedef BOOL (WINAPI *GetSystemPowerStatusFunc)(LPSYSTEM_POWER_STATUS);


struct CRenderTargetList
{
	int					width;
	int					height;
	void*	pTex;
	void*	pDepth;
	CRenderTargetList*	next;
};

struct CTextureList
{
	CGPUTextureId		tex;
	CGPUTextureViewId	tex_view;
	uint8_t*			locked;
	CTextureList*		next;
};

struct CResourceList
{
	char				filename[_MAX_PATH];
	char				password[64];
	CResourceList*		next;
};

struct CStreamList
{
	HSTREAM				hstream;
	void*				data;
	CStreamList*		next;
};

struct CShader
{
	CGPUShaderEntryDescriptor entry[2];
	CGPURootSignatureId root_sigs;
};

struct CInputEventList
{
	hgeInputEvent		event;
	CInputEventList*	next;
};

struct CVertexBufferList
{
	CGPUBufferId pVB;
	CGPUBufferId pIB;
	uint32_t vb_eaten;
	uint32_t ib_eaten;
	CVertexBufferList* next;
};

struct PerSwapChainInfo
{
	CGPUTextureId texture;
	CGPUTextureViewId texture_view;
	CGPUFramebufferId framebuffer;
};

struct PerFrameData
{
	CGPUFenceId inflight_fence{ CGPU_NULLPTR };
	CGPUSemaphoreId prepared_semaphore{ CGPU_NULLPTR };
	CGPUCommandPoolId pool{ CGPU_NULLPTR };
	std::vector<CGPUCommandBufferId> cmds;
	std::vector<CGPUCommandBufferId> allocated_cmds;
	std::vector<CGPUDescriptorSetId> allocated_descriptor_sets;
	CGPUDescriptorSetId last_descriptor_set{ CGPU_NULLPTR };
};

struct PerFrameUBOData
{
	glm::mat4 matProj;
	glm::mat4 matView;
};

struct DescriptorSetKey
{
	CTextureList* tex;
	bool sampler;
	bool color;

	bool operator==(const DescriptorSetKey& other) const {
		return tex == other.tex && sampler == other.sampler && color == other.color;
	}
};

struct DescriptorSetKeyHash
{
	std::size_t operator()(const DescriptorSetKey& k) const {
		return ((std::hash<void*>()(k.tex) ^ (std::hash<bool>()(k.sampler) << 1)) << 1) ^ (std::hash<bool>()(k.color) << 1);
	}
};

void DInit();
void DDone();
bool DFrame(HGE* hge, void* userdata);
bool DRender(HGE* hge, void* userdata);


/*
** HGE Interface implementation
*/
class HGE_Impl : public HGE
{
public:
	virtual	void		CALL	Release();

	virtual bool		CALL	System_Initiate();
	virtual void		CALL	System_Shutdown();
	virtual bool		CALL	System_Start();
	virtual void		CALL	System_SetStateBool  (hgeBoolState   state, bool        value);
	virtual void		CALL	System_SetStateFunc  (hgeFuncState   state, hgeCallback value);
	virtual void		CALL	System_SetStateHwnd  (hgeHwndState   state, HWND        value);
	virtual void		CALL	System_SetStateInt   (hgeIntState    state, int         value);
	virtual void		CALL	System_SetStateString(hgeStringState state, const char *value);
	virtual bool		CALL	System_GetStateBool  (hgeBoolState  );
	virtual hgeCallback	CALL	System_GetStateFunc  (hgeFuncState  );
	virtual HWND		CALL	System_GetStateHwnd  (hgeHwndState  );
	virtual int			CALL	System_GetStateInt   (hgeIntState   );
	virtual const char*	CALL	System_GetStateString(hgeStringState);
	virtual char*		CALL	System_GetErrorMessage();
	virtual	void		CALL	System_Log(const char *format, ...);
	virtual bool		CALL	System_Launch(const char *url);
	virtual void		CALL	System_Snapshot(const char *filename=0);

	virtual void*		CALL	Resource_Load(const char *filename, DWORD *size=0);
	virtual void		CALL	Resource_Free(void *res);
	virtual bool		CALL	Resource_AttachPack(const char *filename, const char *password=0);
	virtual void		CALL	Resource_RemovePack(const char *filename);
	virtual void		CALL	Resource_RemoveAllPacks();
	virtual char*		CALL	Resource_MakePath(const char *filename=0);
	virtual char*		CALL	Resource_EnumFiles(const char *wildcard=0);
	virtual char*		CALL	Resource_EnumFolders(const char *wildcard=0);

	virtual	void		CALL	Ini_SetInt(const char *section, const char *name, int value);
	virtual	int 		CALL	Ini_GetInt(const char *section, const char *name, int def_val);
	virtual	void		CALL	Ini_SetFloat(const char *section, const char *name, float value);
	virtual	float		CALL	Ini_GetFloat(const char *section, const char *name, float def_val);
	virtual	void		CALL	Ini_SetString(const char *section, const char *name, const char *value);
	virtual	char*		CALL	Ini_GetString(const char *section, const char *name, const char *def_val);

	virtual void		CALL	Random_Seed(int seed=0);
	virtual int			CALL	Random_Int(int min, int max);
	virtual float		CALL	Random_Float(float min, float max);

	virtual float		CALL	Timer_GetTime();
	virtual float		CALL	Timer_GetDelta();
	virtual int			CALL	Timer_GetFPS();

	virtual HEFFECT		CALL	Effect_Load(const char *filename, DWORD size=0);
	virtual void		CALL	Effect_Free(HEFFECT eff);
	virtual HCHANNEL	CALL 	Effect_Play(HEFFECT eff);
	virtual HCHANNEL	CALL 	Effect_PlayEx(HEFFECT eff, int volume=100, int pan=0, float pitch=1.0f, bool loop=false);

	virtual HMUSIC		CALL 	Music_Load(const char *filename, DWORD size=0);
	virtual void		CALL	Music_Free(HMUSIC mus);
	virtual HCHANNEL	CALL 	Music_Play(HMUSIC mus, bool loop, int volume = 100, int order = 0, int row = 0);
	virtual void		CALL	Music_SetAmplification(HMUSIC music, int ampl);
	virtual int			CALL	Music_GetAmplification(HMUSIC music);
	virtual int			CALL	Music_GetLength(HMUSIC music);
	virtual void		CALL	Music_SetPos(HMUSIC music, int order, int row);
	virtual bool		CALL	Music_GetPos(HMUSIC music, int *order, int *row);
	virtual void		CALL	Music_SetInstrVolume(HMUSIC music, int instr, int volume);
	virtual int			CALL	Music_GetInstrVolume(HMUSIC music, int instr);
	virtual void		CALL	Music_SetChannelVolume(HMUSIC music, int channel, int volume);
	virtual int			CALL	Music_GetChannelVolume(HMUSIC music, int channel);

	virtual HSTREAM		CALL	Stream_Load(const char *filename, DWORD size=0);
	virtual void		CALL	Stream_Free(HSTREAM stream);
	virtual HCHANNEL	CALL	Stream_Play(HSTREAM stream, bool loop, int volume = 100);

	virtual void		CALL 	Channel_SetPanning(HCHANNEL chn, int pan);
	virtual void		CALL 	Channel_SetVolume(HCHANNEL chn, int volume);
	virtual void		CALL 	Channel_SetPitch(HCHANNEL chn, float pitch);
	virtual void		CALL 	Channel_Pause(HCHANNEL chn);
	virtual void		CALL 	Channel_Resume(HCHANNEL chn);
	virtual void		CALL 	Channel_Stop(HCHANNEL chn);
	virtual void		CALL 	Channel_PauseAll();
	virtual void		CALL 	Channel_ResumeAll();
	virtual void		CALL 	Channel_StopAll();
	virtual bool		CALL	Channel_IsPlaying(HCHANNEL chn);
	virtual float		CALL	Channel_GetLength(HCHANNEL chn);
	virtual float		CALL	Channel_GetPos(HCHANNEL chn);
	virtual void		CALL	Channel_SetPos(HCHANNEL chn, float fSeconds);
	virtual void		CALL	Channel_SlideTo(HCHANNEL channel, float time, int volume, int pan = -101, float pitch = -1);
	virtual bool		CALL	Channel_IsSliding(HCHANNEL channel);

	virtual void		CALL	Input_GetMousePos(float *x, float *y);
	virtual void		CALL	Input_SetMousePos(float x, float y);
	virtual int			CALL	Input_GetMouseWheel();
	virtual bool		CALL	Input_IsMouseOver();
	virtual bool		CALL	Input_KeyDown(int key);
	virtual bool		CALL	Input_KeyUp(int key);
	virtual bool		CALL	Input_GetKeyState(int key);
	virtual const char*	CALL	Input_GetKeyName(int key);
	virtual int			CALL	Input_GetKey();
	virtual int			CALL	Input_GetChar();
	virtual bool		CALL	Input_GetEvent(hgeInputEvent *event);

	virtual bool		CALL	Gfx_BeginScene(HTARGET target=0);
	virtual void		CALL	Gfx_EndScene();
	virtual void		CALL	Gfx_Clear(DWORD color);
	virtual void		CALL	Gfx_RenderLine(float x1, float y1, float x2, float y2, DWORD color=0xFFFFFFFF, float z=0.5f);
	virtual void		CALL	Gfx_RenderTriple(const hgeTriple *triple);
	virtual void		CALL	Gfx_RenderQuad(const hgeQuad *quad);
	virtual hgeVertex*	CALL	Gfx_StartBatch(int prim_type, HTEXTURE tex, int blend, int *max_prim);
	virtual void		CALL	Gfx_FinishBatch(int nprim);
	virtual void		CALL	Gfx_SetClipping(int x=0, int y=0, int w=0, int h=0);
	virtual void		CALL	Gfx_SetTransform(float x=0, float y=0, float dx=0, float dy=0, float rot=0, float hscale=0, float vscale=0); 

	virtual HTARGET		CALL	Target_Create(int width, int height, bool zbuffer);
	virtual void		CALL	Target_Free(HTARGET target);
	virtual HTEXTURE	CALL	Target_GetTexture(HTARGET target);

	virtual HTEXTURE	CALL	Texture_Create(int width, int height);
	virtual HTEXTURE	CALL	Texture_Load(const char *filename, DWORD size=0, bool bMipmap=false);
	virtual void		CALL	Texture_Free(HTEXTURE tex);
	virtual int			CALL	Texture_GetWidth(HTEXTURE tex, bool bOriginal=false);
	virtual int			CALL	Texture_GetHeight(HTEXTURE tex, bool bOriginal=false);
	virtual DWORD*		CALL	Texture_Lock(HTEXTURE tex, bool bReadOnly=true, int left=0, int top=0, int width=0, int height=0);
	virtual void		CALL	Texture_Unlock(HTEXTURE tex);

	virtual void		CALL	JS_Start(hgeJobThreadCallback thread_start, hgeJobThreadCallback thread_end);
	virtual void		CALL	JS_Shutdown();
	virtual int			CALL	JS_GetWorkerThreadCount();
	virtual int			CALL	JS_GetThreadId();
	virtual HJOB		CALL	JS_CreateJob(HJOB parent);
	virtual HJOB		CALL	JS_CreateJob(hgeJobCallback jobCallback, const hgeJobPayload& payload, HJOB parent);
	virtual void		CALL	JS_Run(HJOB job);
	virtual void		CALL	JS_RunAndWait(HJOB job);

	//////// Implementation ////////

	static HGE_Impl*	_Interface_Get();
	void				_FocusChange(bool bAct);
	void				_PostError(const char *error);


	HINSTANCE			hInstance;
	HWND				hwnd;

	bool				bActive;
	char				szError[256];
	char				szCurPath[_MAX_PATH];
	char				szIniString[256];


	// System States
	hgeCallback			procFrameFunc;
	hgeCallback			procRenderFunc;
	hgeCallback			procFocusLostFunc;
	hgeCallback			procFocusGainFunc;
	hgeCallback			procGfxRestoreFunc;
	hgeCallback			procExitFunc;
	const char*			szIcon;
	char				szWinTitle[256];
	int					nScreenWidth;
	int					nScreenHeight;
	int					nScreenBPP;
	bool				bWindowed;
	bool				bZBuffer;
	bool				bTextureFilter;
	char				szIniFile[_MAX_PATH];
	char				szLogFile[_MAX_PATH];
	bool				bUseSound;
	int					nSampleRate;
	int					nFXVolume;
	int					nMusVolume;
	int					nStreamVolume;
	int					nHGEFPS;
	bool				bHideMouse;
	bool				bDontSuspend;
	bool				bEnableCaptureRender;
	HWND				hwndParent;

	#ifdef DEMO
	bool				bDMO;
	#endif


	// Power
	int							nPowerStatus;
	HMODULE						hKrnl32;
	GetSystemPowerStatusFunc	lpfnGetSystemPowerStatus;

	void				_InitPowerStatus();
	void				_UpdatePowerStatus();
	void				_DonePowerStatus();


	// Graphics
	CGPUInstanceId instance;
	CGPUDeviceId device;
	CGPUQueueId gfx_queue;
	CGPUQueueId present_queue;
	CGPUSurfaceId surface;
	CGPUSwapChainId swapchain;
	RECT rectW;
	LONG styleW;
	RECT rectFS;
	LONG styleFS;
	std::vector<PerSwapChainInfo> swapchain_infos;
	uint32_t current_swapchain_index;
	CGPURenderPassId render_pass;
	std::vector<PerFrameData> frame_datas;
	uint32_t current_frame_index;
	CGPUSemaphoreId render_finished_semaphore;
	CGPUCommandBufferId cur_cmd;
	CGPURenderPassEncoderId cur_rp_encoder;
	bool rendering;
	bool prepared;
	CVertexBufferList* cur_vertex_buffer;
	CShader default_shaders[2];
	std::unordered_map<uint32_t, CGPURenderPipelineId> default_shader_pipelines;
	std::unordered_map<const CTextureList*, std::unordered_map<uint32_t, CGPUDescriptorSetId>> default_shader_descriptor_sets;
	CGPUSamplerId linear_sampler, point_sampler;
	std::vector<CTextureList*> deleted_textures;
	CGPUBufferId per_frame_ubo;
	CGPUDescriptorSetId per_frame_ubo_descriptor_sets[2];

	glm::mat4			matView;
	glm::mat4			matProj;

	CTextureList*		textures;
	CVertexBufferList*	vertexBuffers;
	hgeVertex*			VertArray;
	CTextureList*		tex_white;

	int					nPrim;
	int					CurPrimType;
	int					CurBlendMode;
	const CTextureList*		CurTexture;
	CGPURenderPipelineId CurDefaultShaderPipeline;
	CGPUDescriptorSetId CurDefaultDescriptorSet;

	bool				_GfxInit();
	void				_GfxDone();
	bool				_GfxStart();
	void				_GfxEnd();
	bool				_GfxRestore();
	void				_AdjustWindow();
	void				_Resize(int width, int height);
	bool				_init_lost();
	void				_render_batch(bool bEndScene=false);
	void				_SetBlendMode(int blend);
	void				_SetProjectionMatrix(int width, int height);
	CGPUCommandBufferId	_RequestCmd(PerFrameData &frame_data);
	CGPURenderPipelineId _RequestPipeline(int primType, bool blend, bool color);
	CGPUDescriptorSetId _RequestDescriptorSet(const CTextureList* texItem, bool sampler, bool color);
	void				_DeleteDescriptorSet(const CTextureList* texItem);
	bool				_OutOfVertexBugets(uint32_t request_vertex_count, uint32_t request_index_count);
	void				_UploadVertexData(const hgeVertex* v);
	void				_ExpandVertexBuffer();
	void				_RenderPrim(int prim_type, const hgeVertex* v, const CTextureList* tex, int blend);
	void				_FreeDeletedTextures();

	// Render Capture
	RENDERDOC_API_1_0_0*	rdc = nullptr;
	bool				rdc_capture = false;

	void				_CaptureInit();
	void				_CaptureStart();
	void				_CaptureEnd();

	// Audio
	HINSTANCE			hBass;
	bool				bSilent;
	CStreamList*		streams;
	bool				_SoundInit();
	void				_SoundDone();
	void				_SetMusVolume(int vol);
	void				_SetStreamVolume(int vol);
	void				_SetFXVolume(int vol);


	// Input
	int					VKey;
	int					Char;
	int					Zpos;
	float				Xpos;
	float				Ypos;
	bool				bMouseOver;
	bool				bCaptured;
	char				keyz[256];
	CInputEventList*	queue;
	void				_UpdateMouse();
	void				_InputInit();
	void				_ClearQueue();
	void				_BuildEvent(int type, int key, int scan, int flags, int x, int y);


	// Resources
	char				szTmpFilename[_MAX_PATH];
	CResourceList*		res;
	HANDLE				hSearch;
	WIN32_FIND_DATA		SearchData;


	// Timer
	float				fTime;
	float				fDeltaTime;
	DWORD				nFixedDelta;
	int					nFPS;
	DWORD				t0, t0fps, dt;
	int					cfps;


	// Job System
	utils::JobSystem*	pJobSystem;

private:
	HGE_Impl();
};

extern HGE_Impl*		pHGE;

#endif

