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

static NativeSymbol hge_symbols[] = {
    { "System_SetState", System_SetState, "(i$)", NULL },
    { "Input_GetKeyState", Input_GetKeyState, "(i)i", NULL },
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