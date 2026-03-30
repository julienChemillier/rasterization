#ifndef POINTS_HPP
#define POINTS_HPP

#include <cmath>

// -------------------------------------------------------
// ----------------------- Float 2 -----------------------
// -------------------------------------------------------

struct float2 {
    float x;
    float y;

    // Opération '-'
    float2 operator-(const float2& other) const {
        return {x - other.x, y - other.y};
    }

    // Opération '+'
    float2 operator+(const float2& other) const {
        return {x + other.x, y + other.y};
    }

    // Opération '* scalar'
    float2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }
};

// Renvoie le produit scalaire de a et b
inline float dot(float2 a, float2 b) {
    return a.x * b.x + a.y * b.y;
}

// Renvoie une rotation de 90° (sens anti-trigo) du vecteur a
inline float2 perpendicular(float2 a) {
    return {a.y, -a.x};
}

// Renvoie True ssi p est du côté droit du vecteur (v1, v2)
inline bool points_on_right_side(float2 v1, float2 v2, float2 p) {
    float2 v1p = p - v1;
    float2 v12Per = perpendicular(v2 - v1);
    return dot(v1p, v12Per) >= 0;
}

// -------------------------------------------------------
// ----------------------- Float 3 -----------------------
// -------------------------------------------------------

struct float3 {
    float x;
    float y;
    float z;

    // Operation '-'
    float3 operator-(const float3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    // Operation '+'
    float3 operator+(const float3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    // Operation '* scalar'
    float3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    // --- AJOUTS POUR LE SHADING ---

    // Produit scalaire (Dot Product)
    float dot(const float3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Produit vectoriel (Cross Product)
    float3 cross(const float3& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    // Renvoie la longueur du vecteur
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Renvoie un nouveau vecteur normalisé (longueur = 1)
    float3 normalized() const {
        float len = length();
        if (len > 0.00001f) {
            return {x / len, y / len, z / len};
        }
        return {0, 0, 0}; // Éviter division par zéro
    }
};

#endif