#include "hge.h"

// Quad is the basic primitive in HGE
// used for rendering graphics.
// Quad contains 4 vertices, numbered
// 0 to 3 clockwise.
hgeQuad quad;

// Some "gameplay" variables and constants
float x = 100.0f, y = 100.0f;
float dx = 0.0f, dy = 0.0f;

const float speed = 90;
const float friction = 0.98f;

[[clang::export_name("_app_config")]]
void config()
{
    hge_system_set_state(HGE_TITLE, "HGE WASM Tutorial 02 - Using input, sound and rendering");
}


[[clang::export_name("_app_init")]]
bool init()
{
    quad.tex=hge_texture_load("particles.png");
    quad.blend = BLEND_ALPHAADD | BLEND_COLORMUL | BLEND_ZWRITE;

    for (int i = 0; i < 4; i++)
    {
        // Set up z-coordinate of vertices
        quad.v[i].z = 0.5f;
        // Set up color. The format of DWORD col is 0xAABBGGRR
        quad.v[i].col = ARGB(0xFF, 0xFF, 0xA0, 0x00);
    }

    // Set up quad's texture coordinates.
    // 0,0 means top left corner and 1,1 -
    // bottom right corner of the texture.
    quad.v[0].tx = 96.0 / 128.0; quad.v[0].ty = 64.0 / 128.0;
    quad.v[1].tx = 128.0 / 128.0; quad.v[1].ty = 64.0 / 128.0;
    quad.v[2].tx = 128.0 / 128.0; quad.v[2].ty = 96.0 / 128.0;
    quad.v[3].tx = 96.0 / 128.0; quad.v[3].ty = 96.0 / 128.0;

	if(!quad.tex)
	{
		// If one of the data files is not found, display
		// an error message and shutdown.
		// MessageBox(NULL, "Can't load MENU.WAV or PARTICLES.PNG", "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
		return true;
	}

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

    // Set up quad's screen coordinates
    quad.v[0].x = x - 16; quad.v[0].y = y - 16;
    quad.v[1].x = x + 16; quad.v[1].y = y - 16;
    quad.v[2].x = x + 16; quad.v[2].y = y + 16;
    quad.v[3].x = x - 16; quad.v[3].y = y + 16;

    return false;
}

[[clang::export_name("_app_render")]]
bool render()
{
    // Begin rendering quads.
    // This function must be called
    // before any actual rendering.
    hge_gfx_begin_scene();

    // Clear screen with black color
    hge_gfx_clear(0);

    // Render quads here. This time just
    // one of them will serve our needs.
    hge_gfx_render_quad(&quad);

    // End rendering and update the screen
    hge_gfx_end_scene();

    // RenderFunc should always return false
    return false;
}

[[clang::export_name("_app_exit")]]
void exit()
{
    hge_texture_free(quad.tex);
}
