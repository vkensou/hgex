#include "hgex_wrapper.h"

#include "wasm_export.h"
#include "hge.h"

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