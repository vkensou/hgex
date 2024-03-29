#pragma once

#include <stdint.h>

/*
** Common data types
*/
#ifndef DWORD
typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef unsigned char       BYTE;
#endif

typedef uint64_t HRESOURCE;
typedef uint64_t HTEXTURE;
typedef uint64_t HTARGET;
typedef uint64_t HEFFECT;
typedef uint64_t HMUSIC;
typedef uint64_t HSTREAM;
typedef uint64_t HCHANNEL;
typedef uint64_t HJOB;

struct hgeJobPayload
{
	template<class T>
	hgeJobPayload(const T& t)
	{
		static_assert(sizeof(T) <= sizeof(data), "data too large");
		new (&data) T(t);
	}

	template<class T>
	T cast() const
	{
		return *(reinterpret_cast<const T*>(data));
	}

	uint64_t data;
};

typedef void(*JobCallback)(HJOB, hgeJobPayload);

/*
** Hardware color macros
*/
#define ARGB(a,r,g,b)	((DWORD(a)<<24) + (DWORD(r)<<0) + (DWORD(g)<<8) + (DWORD(b)<<16))
#define RGBA(r,g,b,a)	((DWORD(r)<<0) + (DWORD(g)<<8) + (DWORD(b)<<16) + (DWORD(a)<<24))
#define GETA(col)		((col)>>24)
#define GETR(col)		(((col)>>0) & 0xFF)
#define GETG(col)		(((col)>>8) & 0xFF)
#define GETB(col)		(((col)>>16) & 0xFF)
#define SETA(col,a)		(((col) & 0x00FFFFFF) + (DWORD(a)<<24))
#define SETR(col,r)		(((col) & 0xFFFFFF00) + (DWORD(r)<<0))
#define SETG(col,g)		(((col) & 0xFFFF00FF) + (DWORD(g)<<8))
#define SETB(col,b)		(((col) & 0xFF00FFFF) + (DWORD(b)<<16))


/*
** HGE Blending constants
*/
#define	BLEND_COLORADD		1
#define	BLEND_COLORMUL		0
#define	BLEND_ALPHABLEND	2
#define	BLEND_ALPHAADD		0
#define	BLEND_ZWRITE		4
#define	BLEND_NOZWRITE		0

#define BLEND_DEFAULT		(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE)
#define BLEND_DEFAULT_Z		(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_ZWRITE)

enum hgeStringState
{
	HGE_ICON = 26,   // char*	icon resource		(default: NULL)
	HGE_TITLE = 27,   // char*	window title		(default: "HGE")

	HGE_INIFILE = 28,   // char*	ini file			(default: NULL) (meaning no file)
	HGE_LOGFILE = 29,   // char*	log file			(default: NULL) (meaning no file)

	HGESTRINGSTATE_FORCE_DWORD = 0x7FFFFFFF
};

struct hgeVertex
{
	float			x, y;		// screen position    
	float			z;			// Z-buffer depth 0..1
	uint32_t			col;		// color
	float			tx, ty;		// texture coordinates
};


/*
** HGE Triple structure
*/
struct hgeTriple
{
	hgeVertex		v[3];
	HTEXTURE		tex;
	int				blend;
};


/*
** HGE Quad structure
*/
struct hgeQuad
{
	hgeVertex		v[4];
	HTEXTURE		tex;
	int				blend;
};

[[clang::import_module("hge"), clang::import_name("System_SetState")]]
void hge_system_set_state(hgeStringState state, const char* value);

[[clang::import_module("hge"), clang::import_name("Timer_GetTime")]]
float hge_timer_get_time();
[[clang::import_module("hge"), clang::import_name("Timer_GetDelta")]]
float hge_timer_get_delta();
[[clang::import_module("hge"), clang::import_name("Timer_GetFPS")]]
int hge_timer_get_fps();

[[clang::import_module("hge"), clang::import_name("Input_GetKeyState")]]
bool hge_input_get_key_state(int key);

[[clang::import_module("hge"), clang::import_name("Gfx_BeginScene")]]
bool hge_gfx_begin_scene(HTARGET target = 0);
[[clang::import_module("hge"), clang::import_name("Gfx_EndScene")]]
void hge_gfx_end_scene();
[[clang::import_module("hge"), clang::import_name("Gfx_Clear")]]
void hge_gfx_clear(DWORD color);
[[clang::import_module("hge"), clang::import_name("Gfx_RenderQuad")]]
void hge_gfx_render_quad(const hgeQuad* quad);

[[clang::import_module("hge"), clang::import_name("Texture_Load")]]
HTEXTURE hge_texture_load(const char *filename, uint32_t size=0, bool bMipmap=false);
[[clang::import_module("hge"), clang::import_name("Texture_Free")]]
void hge_texture_free(HTEXTURE tex);

[[clang::import_module("hge"), clang::import_name("Log_Printf")]]
void hge_log_printf(const char* format, ...);

[[clang::import_module("hge"), clang::import_name("JS_CreateEmptyJob")]]
HJOB hge_js_create_empty_job(HJOB job = 0);
[[clang::import_module("hge"), clang::import_name("JS_CreateJob")]]
HJOB hge_js_create_job(HJOB job, JobCallback callback, uint8_t* payload);
inline HJOB hge_js_create_job(HJOB job, JobCallback callback, const hgeJobPayload& payload=nullptr)
{
	return hge_js_create_job(job, callback, (uint8_t*)& payload.data);
}
[[clang::import_module("hge"), clang::import_name("JS_Run")]]
void hge_js_run(HJOB job);
[[clang::import_module("hge"), clang::import_name("JS_RunAndWait")]]
void hge_js_run_and_wait(HJOB job);

/*
** HGE Virtual-key codes
*/
#define HGEK_LBUTTON	0x01
#define HGEK_RBUTTON	0x02
#define HGEK_MBUTTON	0x04

#define HGEK_ESCAPE		0x1B
#define HGEK_BACKSPACE	0x08
#define HGEK_TAB		0x09
#define HGEK_ENTER		0x0D
#define HGEK_SPACE		0x20

#define HGEK_SHIFT		0x10
#define HGEK_CTRL		0x11
#define HGEK_ALT		0x12

#define HGEK_LWIN		0x5B
#define HGEK_RWIN		0x5C
#define HGEK_APPS		0x5D

#define HGEK_PAUSE		0x13
#define HGEK_CAPSLOCK	0x14
#define HGEK_NUMLOCK	0x90
#define HGEK_SCROLLLOCK	0x91

#define HGEK_PGUP		0x21
#define HGEK_PGDN		0x22
#define HGEK_HOME		0x24
#define HGEK_END		0x23
#define HGEK_INSERT		0x2D
#define HGEK_DELETE		0x2E

#define HGEK_LEFT		0x25
#define HGEK_UP			0x26
#define HGEK_RIGHT		0x27
#define HGEK_DOWN		0x28

#define HGEK_0			0x30
#define HGEK_1			0x31
#define HGEK_2			0x32
#define HGEK_3			0x33
#define HGEK_4			0x34
#define HGEK_5			0x35
#define HGEK_6			0x36
#define HGEK_7			0x37
#define HGEK_8			0x38
#define HGEK_9			0x39

#define HGEK_A			0x41
#define HGEK_B			0x42
#define HGEK_C			0x43
#define HGEK_D			0x44
#define HGEK_E			0x45
#define HGEK_F			0x46
#define HGEK_G			0x47
#define HGEK_H			0x48
#define HGEK_I			0x49
#define HGEK_J			0x4A
#define HGEK_K			0x4B
#define HGEK_L			0x4C
#define HGEK_M			0x4D
#define HGEK_N			0x4E
#define HGEK_O			0x4F
#define HGEK_P			0x50
#define HGEK_Q			0x51
#define HGEK_R			0x52
#define HGEK_S			0x53
#define HGEK_T			0x54
#define HGEK_U			0x55
#define HGEK_V			0x56
#define HGEK_W			0x57
#define HGEK_X			0x58
#define HGEK_Y			0x59
#define HGEK_Z			0x5A

#define HGEK_GRAVE		0xC0
#define HGEK_MINUS		0xBD
#define HGEK_EQUALS		0xBB
#define HGEK_BACKSLASH	0xDC
#define HGEK_LBRACKET	0xDB
#define HGEK_RBRACKET	0xDD
#define HGEK_SEMICOLON	0xBA
#define HGEK_APOSTROPHE	0xDE
#define HGEK_COMMA		0xBC
#define HGEK_PERIOD		0xBE
#define HGEK_SLASH		0xBF

#define HGEK_NUMPAD0	0x60
#define HGEK_NUMPAD1	0x61
#define HGEK_NUMPAD2	0x62
#define HGEK_NUMPAD3	0x63
#define HGEK_NUMPAD4	0x64
#define HGEK_NUMPAD5	0x65
#define HGEK_NUMPAD6	0x66
#define HGEK_NUMPAD7	0x67
#define HGEK_NUMPAD8	0x68
#define HGEK_NUMPAD9	0x69

#define HGEK_MULTIPLY	0x6A
#define HGEK_DIVIDE		0x6F
#define HGEK_ADD		0x6B
#define HGEK_SUBTRACT	0x6D
#define HGEK_DECIMAL	0x6E

#define HGEK_F1			0x70
#define HGEK_F2			0x71
#define HGEK_F3			0x72
#define HGEK_F4			0x73
#define HGEK_F5			0x74
#define HGEK_F6			0x75
#define HGEK_F7			0x76
#define HGEK_F8			0x77
#define HGEK_F9			0x78
#define HGEK_F10		0x79
#define HGEK_F11		0x7A
#define HGEK_F12		0x7B
