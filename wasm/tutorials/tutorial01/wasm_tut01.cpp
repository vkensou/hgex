#include "hge.h"

[[clang::export_name("_app_config")]]
void config()
{
    hge_system_set_state(HGE_TITLE, "HGE WASM Tutorial 01 - Minimal HGE application");
}

[[clang::export_name("_app_frame")]]
bool frame()
{
    if (hge_input_get_key_state(HGEK_ESCAPE)) return true;

    return false;
}
