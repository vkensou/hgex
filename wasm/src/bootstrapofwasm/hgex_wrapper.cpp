#include "hgex_wrapper.h"

#include "wasm_export.h"
#include "hge.h"

void System_SetState(wasm_exec_env_t exec_env, int state, const char* str)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    hge->System_SetState((hgeStringState)state, str);
}

bool Input_GetKeyState(wasm_exec_env_t exec_env, int key)
{
    auto hge = (HGE*)wasm_runtime_get_function_attachment(exec_env);
    return hge->Input_GetKeyState(key);
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
    { "Input_GetKeyState", Input_GetKeyState, "(i)i", NULL },
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