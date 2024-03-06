#include "hge.h"

hgeQuad quad;

[[clang::export_name("_app_config")]]
void config()
{
    hge_system_set_state(HGE_TITLE, "HGE WASM Tutorial 02 - Using input, sound and rendering");
}


[[clang::export_name("_app_init")]]
void init()
{
    quad.tex=hge_texture_load("particles.png");
}

[[clang::export_name("_app_frame")]]
bool frame()
{
    if (hge_input_get_key_state(HGEK_ESCAPE)) return true;

    return false;
}

[[clang::export_name("_app_exit")]]
void exit()
{
    hge_texture_free(quad.tex);
}
