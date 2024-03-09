#pragma once

#include "hge.h"
#include "hgesprite.h"

typedef uint64_t hgeParticleSystem;
// class hgeParticle;

[[clang::import_module("hge"), clang::import_name("ParticleSystem_New")]]
hgeParticleSystem hge_particle_system_new(const char *filename, hgeSprite sprite);

[[clang::import_module("hge"), clang::import_name("ParticleSystem_Delete")]]
void hge_particle_system_delete(hgeParticleSystem particle);

[[clang::import_module("hge"), clang::import_name("ParticleSystem_Fire")]]
void hge_particle_system_fire(hgeParticleSystem particle);

[[clang::import_module("hge"), clang::import_name("ParticleSystem_MoveTo")]]
void hge_particle_system_move_to(hgeParticleSystem particle, float x, float y, bool bMoveParticles = false);

[[clang::import_module("hge"), clang::import_name("ParticleSystem_Update")]]
void hge_particle_system_update(hgeParticleSystem particle, float fDeltaTime);

[[clang::import_module("hge"), clang::import_name("ParticleSystem_Render")]]
void hge_particle_system_render(hgeParticleSystem particle);