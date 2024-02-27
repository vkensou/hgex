#pragma once

#include <string>
#include "renderdoc.h"

std::string locate_renderdoc();
bool load_renderdoc(const std::string& path);
RENDERDOC_API_1_0_0* GetRenderDocApi();