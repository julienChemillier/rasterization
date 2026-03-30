#ifndef MAIN_H
#define MAIN_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_stdinc.h>
#include <sys/types.h>
#include <cstdlib>

#include "../include/image.hpp"
#include "../include/points.hpp"
#include "../include/triangle2.hpp"
#include "../include/world.hpp"

#define CREATE_TEXTURE(width, height) texture = SDL_CreateTexture( \
        renderer, SDL_PIXELFORMAT_ARGB8888, \
        SDL_TEXTUREACCESS_STREAMING, \
        width, \
        height\
    )

// Double de l'air (orienté) formé par A B P
// Si > 0, P est à gauche de AB
// Si < 0, P est à droite de AB
// Si = 0, P est aligné avec AB
inline float edgeFunction(const float2& a, const float2& b, const float2& p);

// Fonction d'approximation d'algorithme d'anti-aliasing qui s'applique sur l'image
void apply_fast_aa(std::vector<uint32_t>& buffer, int width, int height);

// Dessine un caractère sur l'écran à la position choisie
void draw_char(std::vector<uint32_t>& buffer, int width, char c, int start_x, int start_y, uint32_t color);

// Dessine une chaine de caractères sur l'écran à la position choisie
void draw_string(std::vector<uint32_t>& buffer, int width, const std::string& str, int start_x, int start_y, uint32_t color);

// Affiche le mesh
void drawMesh(std::vector<uint32_t>& framebuffer, std::vector<float>& zbuffer, int width, int height, world& my_world, const std::vector<triangle3>& mesh, int shading_mode, bool enable_floor, float fov_degrees, float3 global_light_dir);
#endif