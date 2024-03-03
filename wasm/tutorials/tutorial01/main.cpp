#include "hge.h"

__attribute__((export_name("_app_init")))
void init()
{
    System_SetState(HGE_TITLE, "HGE WASM Tutorial 01 - Minimal HGE application");
}

__attribute__((export_name("_app_frame")))
bool frame()
{
    if (Input_GetKeyState(HGEK_ESCAPE)) return true;

    return false;
}
