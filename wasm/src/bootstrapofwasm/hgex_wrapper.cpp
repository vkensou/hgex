#include "hgex_wrapper.h"

#include "wasm_export.h"
#include "hge.h"
#include "hgesprite.h"
#include "hgefont.h"
#include "hgeparticle.h"

#include <stdarg.h>

void System_SetState(wasm_exec_env_t exec_env, int state, const char* str)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    hge->System_SetState((hgeStringState)state, str);
}

float Timer_GetTime(wasm_exec_env_t exec_env)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    return hge->Timer_GetTime();
}

float Timer_GetDelta(wasm_exec_env_t exec_env)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    return hge->Timer_GetDelta();
}

int Timer_GetFPS(wasm_exec_env_t exec_env)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    return hge->Timer_GetFPS();
}

bool Input_GetKeyState(wasm_exec_env_t exec_env, int key)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    return hge->Input_GetKeyState(key);
}

bool Gfx_BeginScene(wasm_exec_env_t exec_env, uint64_t target)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    return hge->Gfx_BeginScene(target);
}

void Gfx_EndScene(wasm_exec_env_t exec_env)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    hge->Gfx_EndScene();
}

void Gfx_Clear(wasm_exec_env_t exec_env, uint32_t color)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    hge->Gfx_Clear(color);
}

void Gfx_RenderQuad(wasm_exec_env_t exec_env, const hgeQuad* quad)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    hge->Gfx_RenderQuad(quad);
}

uint64_t Texture_Load(wasm_exec_env_t exec_env, const char *filename, uint32_t size, bool bMipmap)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    return hge->Texture_Load(filename, size, bMipmap);
}

void Texture_Free(wasm_exec_env_t exec_env, uint64_t tex)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    hge->Texture_Free(tex);
}

uint64_t Sprite_New(wasm_exec_env_t exec_env, uint64_t tex, float x, float y, float w, float h)
{
    return (uint64_t)new hgeSprite(tex, x, y, w, h);
}

void Sprite_Delete(wasm_exec_env_t exec_env, uint64_t sprite)
{
    delete (hgeSprite*)sprite;
}

void Sprite_Render(wasm_exec_env_t exec_env, uint64_t sprite, float x, float y)
{
    ((hgeSprite*)sprite)->Render(x, y);
}

void Sprite_SetColor(wasm_exec_env_t exec_env, uint64_t sprite, DWORD col, int i)
{
    ((hgeSprite*)sprite)->SetColor(col, i);
}

void Sprite_SetHotSpot(wasm_exec_env_t exec_env, uint64_t sprite, float x, float y)
{
    ((hgeSprite*)sprite)->SetHotSpot(x, y);
}

uint64_t Font_New(wasm_exec_env_t exec_env, const char *filename, bool bMipmap)
{
    return (uint64_t)new hgeFont(filename, bMipmap);
}

void Font_Delete(wasm_exec_env_t exec_env, uint64_t font)
{
    delete (hgeFont*)font;
}

void Font_Printf(wasm_exec_env_t exec_env, uint64_t font, float x, float y, int align, const char *format, va_list va_args)
{
    ((hgeFont*)font)->printf(x, y, align, format, va_args);
}

uint64_t ParticleSystem_New(wasm_exec_env_t exec_env, const char* filename, uint64_t sprite)
{
    return (uint64_t)new hgeParticleSystem(filename, (hgeSprite*)sprite);
}

void ParticleSystem_Delete(wasm_exec_env_t exec_env, uint64_t particle)
{
    delete (hgeParticleSystem*)particle;
}

void ParticleSystem_Fire(wasm_exec_env_t exec_env, uint64_t particle)
{
    ((hgeParticleSystem*)particle)->Fire();
}

void ParticleSystem_MoveTo(wasm_exec_env_t exec_env, uint64_t particle, float x, float y, bool bMoveParticles)
{
    ((hgeParticleSystem*)particle)->MoveTo(x, y, bMoveParticles);
}

void ParticleSystem_Update(wasm_exec_env_t exec_env, uint64_t particle, float fDeltaTime)
{
    ((hgeParticleSystem*)particle)->Update(fDeltaTime);
}

void ParticleSystem_Render(wasm_exec_env_t exec_env, uint64_t particle)
{
    ((hgeParticleSystem*)particle)->Render();
}

static NativeSymbol hge_symbols[] = {
    { "System_SetState", System_SetState, "(i$)", NULL },
    { "Timer_GetTime", Timer_GetTime, "()f", NULL },
    { "Timer_GetDelta", Timer_GetDelta, "()f", NULL },
    { "Timer_GetFPS", Timer_GetFPS, "()i", NULL },
    { "Input_GetKeyState", Input_GetKeyState, "(i)i", NULL },
    { "Gfx_BeginScene", Gfx_BeginScene, "(I)i", NULL },
    { "Gfx_EndScene", Gfx_EndScene, "()", NULL },
    { "Gfx_Clear", Gfx_Clear, "(i)", NULL },
    { "Gfx_RenderQuad", Gfx_RenderQuad, "(*)", NULL },
    { "Texture_Load", Texture_Load, "($ii)I", NULL },
    { "Texture_Free", Texture_Free, "(I)", NULL },

    { "Sprite_New", Sprite_New, "(Iffff)I", NULL },
    { "Sprite_Delete", Sprite_Delete, "(I)", NULL },
    { "Sprite_Render", Sprite_Render, "(Iff)", NULL },
    { "Sprite_SetColor", Sprite_SetColor, "(Iii)", NULL },
    { "Sprite_SetHotSpot", Sprite_SetHotSpot, "(Iff)", NULL },

    { "Font_New", Font_New, "($i)I", NULL },
    { "Font_Delete", Font_Delete, "(I)", NULL },
    { "Font_Printf", Font_Printf, "(Iffi$*)", NULL },

    { "ParticleSystem_New", ParticleSystem_New, "($I)I", NULL },
    { "ParticleSystem_Delete", ParticleSystem_Delete, "(I)", NULL },
    { "ParticleSystem_Fire", ParticleSystem_Fire, "(I)", NULL },
    { "ParticleSystem_MoveTo", ParticleSystem_MoveTo, "(Iffi)", NULL },
    { "ParticleSystem_Update", ParticleSystem_Update, "(If)", NULL },
    { "ParticleSystem_Render", ParticleSystem_Render, "(I)", NULL },
};

int wasm_register_hge_apis(HGE* hge)
{
    int n_hge_symbols = sizeof(hge_symbols) / sizeof(NativeSymbol);
    for (size_t i = 0; i < n_hge_symbols; ++i)
        hge_symbols[i].attachment = hge;
    if (wasm_runtime_register_natives("hge",
        hge_symbols,
        n_hge_symbols)) {
        return n_hge_symbols;
    }

    return 0;
}