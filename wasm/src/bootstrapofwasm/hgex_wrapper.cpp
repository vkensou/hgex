#include "hgex_wrapper.h"

#include "wasm_export.h"
#include "hge.h"

HGE* hge = NULL;

void System_SetState(wasm_exec_env_t exec_env, int state, const char* str)
{
	hge->System_SetState((hgeStringState)state, str);
}

bool Input_GetKeyState(wasm_exec_env_t exec_env, int key)
{
    return hge->Input_GetKeyState(key);
}

uint64_t Texture_Load(wasm_exec_env_t exec_env, const char *filename, uint32_t size, bool bMipmap)
{
    return hge->Texture_Load(filename, size, bMipmap);
}

void Texture_Free(wasm_exec_env_t exec_env, uint64_t tex)
{
    hge->Texture_Free(tex);
}

static NativeSymbol hge_symbols[] = {
    { "System_SetState", System_SetState, "(i$)", NULL },
    { "Input_GetKeyState", Input_GetKeyState, "(i)i", NULL },
    { "Texture_Load", Texture_Load, "($ii)I", NULL },
    { "Texture_Free", Texture_Free, "(I)", NULL },
};

int wasm_register_hge_apis()
{
    int n_hge_symbols = sizeof(hge_symbols) / sizeof(NativeSymbol);
    if (wasm_runtime_register_natives("hge",
        hge_symbols,
        n_hge_symbols)) {
        return n_hge_symbols;
    }

    return 0;
}