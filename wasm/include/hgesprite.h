#pragma once

#include "hge.h"

typedef uint64_t hgeSprite;
// class hgeSprite;

[[clang::import_module("hge"), clang::import_name("Sprite_New")]]
hgeSprite hge_sprite_new(HTEXTURE tex, float x, float y, float w, float h);

[[clang::import_module("hge"), clang::import_name("Sprite_Delete")]]
void hge_sprite_delete(hgeSprite sprite);

[[clang::import_module("hge"), clang::import_name("Sprite_Render")]]
void hge_sprite_render(hgeSprite sprite, float x, float y);

[[clang::import_module("hge"), clang::import_name("Sprite_SetColor")]]
void hge_sprite_set_color(hgeSprite sprite, DWORD col, int i=-1);

[[clang::import_module("hge"), clang::import_name("Sprite_SetHotSpot")]]
void hge_sprite_set_hotspot(hgeSprite sprite, float x, float y);