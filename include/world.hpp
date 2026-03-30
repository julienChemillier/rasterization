#ifndef WORLD_HPP
#define WORLD_HPP

#include <cmath>
#include "points.hpp"
#include "triangle2.hpp"

struct world {
    float3 player_pos;
    float2 rotation; // En radians (x: vertical (vers le haut), y: horizontal())

    // Renvoie un float3: x,y (coords 2D écran), z (profondeur pour le tri 3D)
    float3 projection_to_screen_3d(float3 p) {
        // L'origine devient la caméra
        float3 translated = {
            p.x - player_pos.x,
            p.y - player_pos.y,
            p.z - player_pos.z
        };

        // On projette d'abord horizontalement puis verticalement. En vrai l'ordre importe peu, mais il faut faire l'un après l'autre
        // Rotation horizontale -> autour de l'axe y
        float cos_y = cos(rotation.y);
        float sin_y = sin(rotation.y);
        float3 proj_horiz = {
            cos_y * translated.x + sin_y * translated.z,
            translated.y,
            -sin_y * translated.x + cos_y * translated.z
        };

        // Rotation verticale -> autour de l'axe x
        float cos_x = cos(rotation.x);
        float sin_x = sin(rotation.x);
        float3 proj_vertical = {
            proj_horiz.x,
            cos_x * proj_horiz.y - sin_x * proj_horiz.z,
            sin_x * proj_horiz.y + cos_x * proj_horiz.z
        };

        // Perspective
        float z = proj_vertical.z;
        
        // Le point est derrière la caméra on retourne une projection aléatoire
        if (z < 0.0001f) {
            return {0, 0, z}; 
        }

        return {
            proj_vertical.x / z,
            proj_vertical.y / z,
            z // On conserve la profondeur réelle pour le trie selon Z
        };
    }
};

#endif