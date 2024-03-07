#pragma once

#include "hge.h"

#define HGETEXT_LEFT		0
#define HGETEXT_RIGHT		1
#define HGETEXT_CENTER		2
#define HGETEXT_HORZMASK	0x03

#define HGETEXT_TOP			0
#define HGETEXT_BOTTOM		4
#define HGETEXT_MIDDLE		8
#define HGETEXT_VERTMASK	0x0C

typedef uint64_t hgeFont;
// class hgeFont;

[[clang::import_module("hge"), clang::import_name("Font_New")]]
hgeFont hge_font_new(const char *filename, bool bMipmap=false);

[[clang::import_module("hge"), clang::import_name("Font_Delete")]]
void hge_font_delete(hgeFont font);

[[clang::import_module("hge"), clang::import_name("Font_Printf")]]
void hge_font_printf(hgeFont font, float x, float y, int align, const char *format, ...);