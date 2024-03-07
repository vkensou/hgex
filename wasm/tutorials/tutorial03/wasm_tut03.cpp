#include "hge.h"
#include "hgesprite.h"

hgeSprite			spr;

HTEXTURE			tex;

// Some "gameplay" variables and constants
float x = 100.0f, y = 100.0f;
float dx = 0.0f, dy = 0.0f;

const float speed = 90;
const float friction = 0.98f;

[[clang::export_name("_app_config")]]
void config()
{
    hge_system_set_state(HGE_TITLE, "HGE WASM Tutorial 03 - Using helper classes");
}


[[clang::export_name("_app_init")]]
bool init()
{
    tex=hge_texture_load("particles.png");

	if(!tex)
	{
		// If one of the data files is not found, display
		// an error message and shutdown.
		// MessageBox(NULL, "Can't load MENU.WAV or PARTICLES.PNG", "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return true;
	}

    spr=hge_sprite_new(tex, 96, 64, 32, 32);
    hge_sprite_set_color(spr, ARGB(0xFF,0xFF,0xA0,0x00));
    hge_sprite_set_hotspot(spr, 16, 16);

    return false;
}

// This function plays collision sound with
// parameters based on sprite position and speed
void boom() {
    int pan = int((x - 400) / 4);
    float pitch = (dx * dx + dy * dy) * 0.0005f + 0.2f;
}

[[clang::export_name("_app_frame")]]
bool frame()
{
    // Get the time elapsed since last call of FrameFunc().
    // This will help us to synchronize on different
    // machines and video modes.
    float dt = hge_timer_get_delta();

    // Process keys
    if (hge_input_get_key_state(HGEK_ESCAPE)) return true;
    if (hge_input_get_key_state(HGEK_LEFT)) dx -= speed * dt;
    if (hge_input_get_key_state(HGEK_RIGHT)) dx += speed * dt;
    if (hge_input_get_key_state(HGEK_UP)) dy -= speed * dt;
    if (hge_input_get_key_state(HGEK_DOWN)) dy += speed * dt;

    // Do some movement calculations and collision detection	
    dx *= friction; dy *= friction; x += dx; y += dy;
    if (x > 784) { x = 784 - (x - 784); dx = -dx; boom(); }
    if (x < 16) { x = 16 + 16 - x; dx = -dx; boom(); }
    if (y > 584) { y = 584 - (y - 584); dy = -dy; boom(); }
    if (y < 16) { y = 16 + 16 - y; dy = -dy; boom(); }

    return false;
}

[[clang::export_name("_app_render")]]
bool render()
{
    hge_gfx_begin_scene();
    hge_gfx_clear(0);
    hge_sprite_render(spr, x, y);
    hge_gfx_end_scene();

    return false;
}

[[clang::export_name("_app_exit")]]
void exit()
{
    hge_sprite_delete(spr);
    hge_texture_free(tex);
}
