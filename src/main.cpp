#include "../include/main.hpp"
#include "../include/file_opening.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <string>


// Dimension du simulateur utilisée pour faire les calcules (plus elle est élevée, plus ça coûte cher en calculs)
const int ORIGINAL_WIDTH = 800;
const int ORIGINAL_HEIGHT = 450;

// Fonction de changement de dimension de l'image (peut être modifiée avec la touche 'a' du clavier)
using ResizeFunction = void (*)(std::vector<uint32_t>&, int, int, std::vector<uint32_t>&, int, int);
ResizeFunction resizing_image = resizing_image_nearest_neighbour;

inline float edgeFunction(const float2& a, const float2& b, const float2& p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}


void apply_fast_aa(std::vector<uint32_t>& buffer, int width, int height) {
    std::vector<uint32_t> temp = buffer; 
    
    // On parallélise brutalement pour améliorer les performances
    #pragma omp parallel for
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            
            // On récupère le pixel et ses 4 voisins proches
            uint32_t c = temp[y * width + x];  // (0, 0)
            uint32_t u = temp[(y - 1) * width + x]; // (0, -1)
            uint32_t d = temp[(y + 1) * width + x]; // (0, +1)
            uint32_t l = temp[y * width + x - 1]; // (-1, 0)
            uint32_t r = temp[y * width + x + 1]; // (1, 0)

            // Détection de contours sur la couleur verte (plus vue par l'oeil)
            int cg = (c >> 8) & 0xFF, ug = (u >> 8) & 0xFF, dg = (d >> 8) & 0xFF, lg = (l >> 8) & 0xFF, rg = (r >> 8) & 0xFF;
            int max_g = std::max({cg, ug, dg, lg, rg});
            int min_g = std::min({cg, ug, dg, lg, rg});

            // Si un contour est détecté (trop grande différence de couleur), on floute
            if (max_g - min_g > 60) { 
                // On récupère les couleurs rouge bleue
                uint32_t cr = (c >> 16) & 0xFF, cb = c & 0xFF;
                uint32_t ur = (u >> 16) & 0xFF, ub = u & 0xFF;
                uint32_t dr = (d >> 16) & 0xFF, db = d & 0xFF;
                uint32_t lr = (l >> 16) & 0xFF, lb = l & 0xFF;
                uint32_t rr = (r >> 16) & 0xFF, rb = r & 0xFF;

                // On floute
                uint32_t fr = (cr * 2 + ur + dr + lr + rr) / 6;
                uint32_t fg = (cg * 2 + ug + dg + lg + rg) / 6;
                uint32_t fb = (cb * 2 + ub + db + lb + rb) / 6;

                // On applique la modification
                buffer[y * width + x] = (255 << 24) | (fr << 16) | (fg << 8) | fb;
            }
        }
    }
}

// Police pour écrire les chiffres de 0 à 9 avec le code binaire
const uint8_t font[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b010, 0b100, 0b100}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

void draw_char(std::vector<uint32_t>& buffer, int width, char c, int start_x, int start_y, uint32_t color) {
    const uint8_t* glyph = nullptr;
    // Police pour écrire les lettres suivantes avec le code binaire: S, A, G, O, N, F, V, ,
    static const uint8_t char_S[5] = {0b111, 0b100, 0b111, 0b001, 0b111};
    static const uint8_t char_A[5] = {0b010, 0b101, 0b111, 0b101, 0b101}; 
    static const uint8_t char_G[5] = {0b111, 0b100, 0b101, 0b101, 0b111};
    static const uint8_t char_O[5] = {0b111, 0b101, 0b101, 0b101, 0b111};
    static const uint8_t char_N[5] = {0b111, 0b101, 0b111, 0b101, 0b101}; 
    static const uint8_t char_F[5] = {0b111, 0b100, 0b111, 0b100, 0b100};
    static const uint8_t char_V[5] = {0b101, 0b101, 0b101, 0b101, 0b010};
    static const uint8_t char_colon[5]= {0b000, 0b010, 0b000, 0b010, 0b000};

    // Cherche quel caractère doit être écrit
    switch(c) {
        case 'S': glyph = char_S; break;
        case 'A': glyph = char_A; break;
        case 'G': glyph = char_G; break;
        case 'O': glyph = char_O; break;
        case 'N': glyph = char_N; break;
        case 'F': glyph = char_F; break;
        case 'V': glyph = char_V; break;
        case ':': glyph = char_colon; break;
        default:
            if (c >= '0' && c <= '9') glyph = font[c - '0'];
            break;
    }
    if (!glyph) return;

    // Affiche le caractère 
    #pragma omp parallel for
    for (int dy = 0; dy < 5; ++dy) {
        for (int dx = 0; dx < 3; ++dx) {
            if ((glyph[dy] >> (2 - dx)) & 1) {
                for(int sy=0; sy<2; ++sy) {
                    for(int sx=0; sx<2; ++sx) {
                        buffer[(start_y + dy*2 + sy) * width + (start_x + dx*2 + sx)] = color;
                    }
                }
            }
        }
    }
}

void draw_string(std::vector<uint32_t>& buffer, int width, const std::string& str, int start_x, int start_y, uint32_t color) {
    int cursor_x = start_x;
    for (char c : str) {
        if (c != ' ') draw_char(buffer, width, c, cursor_x, start_y, color);
        cursor_x += 8; 
    }
}

void drawMesh(std::vector<uint32_t>& framebuffer, std::vector<float>& zbuffer, int width, int height, world& my_world, const std::vector<triangle3>& mesh, int shading_mode, bool enable_floor, float fov_degrees, float3 global_light_dir) {
    // Initialisation de la frame
    std::fill(framebuffer.begin(), framebuffer.end(), 0xFF202020);
    std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<float>::max());

    // Calcule la perspective choisie
    float fov_rad = fov_degrees * (M_PI / 180.0f);
    float fov_scale = (ORIGINAL_WIDTH / 2.0f) / std::tan(fov_rad / 2.0f);
    fov_scale *= (width / (float)ORIGINAL_WIDTH); 

    // Calcule le centre de l'écran
    float center_x = width / 2.0f;
    float center_y = height / 2.0f;

    // On ne dessine le sol que si le sol est activé et l'utilisateur est au dessus
    const float floor_y = -1.0f; 
    if (enable_floor && my_world.player_pos.y > floor_y) {
        // Précalcule des cos, sin
        float cos_y = cos(my_world.rotation.y);
        float sin_y = sin(my_world.rotation.y);
        float cos_x = cos(my_world.rotation.x);
        float sin_x = sin(my_world.rotation.x);

        #pragma omp parallel for
        for (int y = 0; y < height; ++y) {
            // On ignore les cases au dessus d'une certaine zone de l'écran
            if (y < center_y - 20) continue; 

            for (int x = 0; x < width; ++x) {
                // On envoie un rayon pour chaque position de l'écran
                float vx = (x - center_x) / fov_scale;
                float vy = -(y - center_y) / fov_scale; 
                float vz = 1.0f;

                // Application de la rotation sur ce rayon
                float tvx = vx;
                float tvy = cos_x * vy + sin_x * vz;
                float tvz = -sin_x * vy + cos_x * vz;
                float ray_dx = cos_y * tvx - sin_y * tvz;
                float ray_dy = tvy;
                float ray_dz = sin_y * tvx + cos_y * tvz;

                // Il point vers le ciel, on l'ignore
                if (ray_dy >= 0) continue;

                // On calcule la distance à laquelle le rayon percute le sol
                float t = (floor_y - my_world.player_pos.y) / ray_dy;
                if (t <= 0) continue;

                // On calcule la position exacte
                float wx = my_world.player_pos.x + t * ray_dx;
                float wz = my_world.player_pos.z + t * ray_dz;

                // Génération du damier noir et blanc
                float scale_floor = 2.0f; 
                bool is_white = ((int)floor(wx * scale_floor) + (int)floor(wz * scale_floor)) % 2 == 0;
                uint8_t color_val = is_white ? 180 : 30; 
                
                // On ajoute une brume en fonction de la distance
                float fog = std::clamp(1.0f - (t / 50.0f), 0.1f, 1.0f);
                color_val = (uint8_t)(color_val * fog);

                // On affiche ce point si c'est le plus proche de l'écran pour ce pixel
                int pixel_index = y * width + x;
                if (t < zbuffer[pixel_index]) {
                    zbuffer[pixel_index] = t;
                    framebuffer[pixel_index] = (255 << 24) | (color_val << 16) | (color_val << 8) | color_val;
                }
            }
        }
    }


    for (size_t i = 0; i < mesh.size(); ++i) {
        const auto& t3 = mesh[i];

        // On calcule la norme du triangle
        float3 edge1 = t3.b - t3.a;
        float3 edge2 = t3.c - t3.a;
        float3 face_normal = edge1.cross(edge2).normalized();
        
        // Si le triangle est à l'envers, on l'affiche pas
        float3 view_dir_face = (t3.a - my_world.player_pos).normalized();
        if (face_normal.dot(view_dir_face) > 0) continue; 

        // On définit la lumièe ambiante et la lumière diffusée
        float i_a = 0, i_b = 0, i_c = 0, face_i = 0;
        float ambient = 0.2f;
        float diffuse_strength = 0.8f;

        if (shading_mode == 1) {
            // On utilise l'ombre de Gouraud, on calcule la lumière sur chacun des sommets du triangle
            i_a = ambient + diffuse_strength * std::max(t3.na.dot(global_light_dir), 0.0f);
            i_b = ambient + diffuse_strength * std::max(t3.nb.dot(global_light_dir), 0.0f);
            i_c = ambient + diffuse_strength * std::max(t3.nc.dot(global_light_dir), 0.0f);
        } else if (shading_mode == 0) {
            // On n'utlise pas d'ombre classique, la lumière est celle d'une seule des positions du triangle
            face_i = ambient + diffuse_strength * std::max(face_normal.dot(global_light_dir), 0.0f);
        }

        // Projection vers la 2D
        float3 p_a = my_world.projection_to_screen_3d(t3.a);
        float3 p_b = my_world.projection_to_screen_3d(t3.b);
        float3 p_c = my_world.projection_to_screen_3d(t3.c);

        // Si l'un des sommets est derrière la caméra, on ignore le triangle
        if (p_a.z < 0.1f || p_b.z < 0.1f || p_c.z < 0.1f) continue;

        // Mise en place du champ de vision et de l'inversion de la hauteur (pour que les objets soient à l'endroit)
        triangle2 screen_t;
        screen_t.a = { p_a.x * fov_scale + center_x, center_y - (p_a.y * fov_scale) };
        screen_t.b = { p_b.x * fov_scale + center_x, center_y - (p_b.y * fov_scale) };
        screen_t.c = { p_c.x * fov_scale + center_x, center_y - (p_c.y * fov_scale) };

        // Calcule l'air, si elle est négative, on inverse pour qu'elle redevienen positive
        float area = edgeFunction(screen_t.a, screen_t.b, screen_t.c);
        if (area < 0) {
            std::swap(screen_t.b, screen_t.c);
            std::swap(p_b.z, p_c.z);
            if (shading_mode == 1) std::swap(i_b, i_c);
            area = -area;
        }
        // Il sera invisible, donc on ignore
        if (area == 0.0f) continue;

        float inv_area = 1.0f / area;
        // Couleur de base de l'objet définie ici (or)
        uint8_t base_r = 255, base_g = 150, base_b = 50;

        // Calcule de la bounding box de l'objet
        int min_x = std::max(0, (int)std::min({screen_t.a.x, screen_t.b.x, screen_t.c.x}));
        int max_x = std::min(width - 1, (int)std::max({screen_t.a.x, screen_t.b.x, screen_t.c.x}));
        int min_y = std::max(0, (int)std::min({screen_t.a.y, screen_t.b.y, screen_t.c.y}));
        int max_y = std::min(height - 1, (int)std::max({screen_t.a.y, screen_t.b.y, screen_t.c.y}));

        // On dessine l'objet
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                float2 p = {(float)x, (float)y};
                float w_c = edgeFunction(screen_t.a, screen_t.b, p);
                float w_a = edgeFunction(screen_t.b, screen_t.c, p);
                float w_b = edgeFunction(screen_t.c, screen_t.a, p);

                // Le pixel est bien dans le triangle si tout est positif
                if (w_a >= 0 && w_b >= 0 && w_c >= 0) {

                    // Récupérer les coordonnées barycentriques
                    w_a *= inv_area; w_b *= inv_area; w_c *= inv_area;

                    // Interpolation de la profondeur exacte du pixel
                    float pixel_z = (w_a * p_a.z) + (w_b * p_b.z) + (w_c * p_c.z);
                    int pixel_index = y * width + x;
                    
                    if (pixel_z < zbuffer[pixel_index]) {
                        zbuffer[pixel_index] = pixel_z;
                        
                        float final_r = 0, final_g = 0, final_b = 0;

                        if (shading_mode == 2) { 
                            // Blinn–Phong reflection

                            // Interpolation sur la surface
                            float3 N = {
                                (t3.na.x * w_a) + (t3.nb.x * w_b) + (t3.nc.x * w_c),
                                (t3.na.y * w_a) + (t3.nb.y * w_b) + (t3.nc.y * w_c),
                                (t3.na.z * w_a) + (t3.nb.z * w_b) + (t3.nc.z * w_c)
                            };
                            N = N.normalized();

                            // Interpolation de la position 3d
                            float3 FragPos = {
                                (t3.a.x * w_a) + (t3.b.x * w_b) + (t3.c.x * w_c),
                                (t3.a.y * w_a) + (t3.b.y * w_b) + (t3.c.y * w_c),
                                (t3.a.z * w_a) + (t3.b.z * w_b) + (t3.c.z * w_c)
                            };

                            // Calculs de l'éclairage
                            float3 view_dir = (my_world.player_pos - FragPos).normalized();
                            float3 half_vec = (global_light_dir + view_dir).normalized();

                            // Composante diffsue
                            float diff = std::max(N.dot(global_light_dir), 0.0f);

                            // Composante réfléchie (specular)
                            float spec = std::pow(std::max(N.dot(half_vec), 0.0f), 64.0f);

                            float intensity = ambient + (diffuse_strength * diff);
                            
                            // Ajoute de l'ombre/lumière
                            final_r = std::clamp((base_r * intensity) + (255 * spec * 0.8f), 0.0f, 255.0f);
                            final_g = std::clamp((base_g * intensity) + (255 * spec * 0.8f), 0.0f, 255.0f);
                            final_b = std::clamp((base_b * intensity) + (255 * spec * 0.8f), 0.0f, 255.0f);

                        } else {
                            // Ombre de Gouraud pour lisser l'objet
                            float pixel_intensity = (shading_mode == 1) ? ((w_a * i_a) + (w_b * i_b) + (w_c * i_c)) : face_i;
                            final_r = base_r * pixel_intensity;
                            final_g = base_g * pixel_intensity;
                            final_b = base_b * pixel_intensity;
                        }
                        
                        // On applique les calculs
                        framebuffer[pixel_index] = (255 << 24) | ((uint8_t)final_r << 16) | ((uint8_t)final_g << 8) | (uint8_t)final_b;
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    int current_width = ORIGINAL_WIDTH;
    int current_height = ORIGINAL_HEIGHT;

    SDL_Window* window = SDL_CreateWindow("Rasterization - Dynamic Light", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, current_width, current_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, current_width, current_height);

    std::vector<uint32_t> original_image(ORIGINAL_WIDTH * ORIGINAL_HEIGHT, 0);
    std::vector<float> z_buffer(ORIGINAL_WIDTH * ORIGINAL_HEIGHT, std::numeric_limits<float>::max());
    std::vector<uint32_t> display_image = original_image; 

    std::vector<float3> vertices;
    std::vector<Face> faces;
    std::vector<triangle3> mesh;
    
    std::string obj_filename = (argc > 1) ? argv[1] : "objects/bimba.obj";

    if (parseOBJ(obj_filename, vertices, faces)) {
        std::vector<float3> vertex_normals(vertices.size(), {0.0f, 0.0f, 0.0f});
        
        for (const auto& face : faces) {
            float3 a = vertices[face.v1 - 1];
            float3 b = vertices[face.v2 - 1];
            float3 c = vertices[face.v3 - 1];
            
            float3 edge1 = b - a;
            float3 edge2 = c - a;
            float3 face_normal = edge1.cross(edge2).normalized();
            
            vertex_normals[face.v1 - 1] = vertex_normals[face.v1 - 1] + face_normal;
            vertex_normals[face.v2 - 1] = vertex_normals[face.v2 - 1] + face_normal;
            vertex_normals[face.v3 - 1] = vertex_normals[face.v3 - 1] + face_normal;
        }
        
        for (auto& n : vertex_normals) n = n.normalized();
        
        for (const auto& face : faces) {
            mesh.push_back({
                vertices[face.v1 - 1], vertices[face.v2 - 1], vertices[face.v3 - 1],
                vertex_normals[face.v1 - 1], vertex_normals[face.v2 - 1], vertex_normals[face.v3 - 1]
            });
        }
    }

    world my_world;
    my_world.player_pos = {0.0f, 0.0f, -5.0f}; 
    my_world.rotation = {0.0f, 0.0f};

    float move_speed = 0.1f; 
    bool running = true;
    SDL_Event event;

    int shading_mode = 2;
    // 0 = Constant
    // 1 = Gouraud
    // 2 = Blinn-Phong

    bool enable_fxaa = true;
    bool enable_floor = true;
    float fov_degrees = 90.0f;

    // Direction de la lumière qui peut changer avec le clavier
    float3 global_light_dir = {0.5f, 1.0f, -0.8f};

    Uint32 last_fps_time = SDL_GetTicks();
    int frame_count = 0;
    int current_fps = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) running = false;
            
            if (event.type == SDL_MOUSEWHEEL) {
                fov_degrees -= event.wheel.y * 5.0f;
                fov_degrees = std::clamp(fov_degrees, 30.0f, 150.0f);
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_g) shading_mode = (shading_mode + 1) % 3;
                if (event.key.keysym.sym == SDLK_h) enable_fxaa = !enable_fxaa;
                if (event.key.keysym.sym == SDLK_p) enable_floor = !enable_floor;
                if (event.key.keysym.sym == SDLK_a) {
                    resizing_image = (resizing_image == resizing_image_bilinear_interpolation) 
                        ? resizing_image_nearest_neighbour : resizing_image_bilinear_interpolation;
                }
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                current_width = event.window.data1;
                current_height = event.window.data2;
                display_image.resize(current_width * current_height);
                SDL_DestroyTexture(texture);
                texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, current_width, current_height);
            }
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);
        
        // Faire bouger la lumière
        float light_speed = 0.05f;
        // J, L pour bouger la lumière selon x (gauche/droite)
        if (state[SDL_SCANCODE_J]) global_light_dir.x -= light_speed;
        if (state[SDL_SCANCODE_L]) global_light_dir.x += light_speed;
        
        // U, O pour bouger la lumière selon y (haut/bas)
        if (state[SDL_SCANCODE_U]) global_light_dir.y += light_speed;
        if (state[SDL_SCANCODE_O]) global_light_dir.y -= light_speed;

        // I, K pour bouger la lumière selon z (avant/arrière)
        if (state[SDL_SCANCODE_I]) global_light_dir.z += light_speed;
        if (state[SDL_SCANCODE_K]) global_light_dir.z -= light_speed;

        // On s'assure que le vecteur lumière est toujours de longueur 1
        float3 normalized_light = global_light_dir.normalized();

        // Faire bouger la caméra
        float yaw = my_world.rotation.y;
        float forward_x = -sin(yaw);
        float forward_z = cos(yaw);
        float right_x = cos(yaw);
        float right_z = sin(yaw);

        if (state[SDL_SCANCODE_UP]) {
            my_world.player_pos.x += forward_x * move_speed;
            my_world.player_pos.z += forward_z * move_speed;
        }
        if (state[SDL_SCANCODE_DOWN]) {
            my_world.player_pos.x -= forward_x * move_speed;
            my_world.player_pos.z -= forward_z * move_speed;
        }
        if (state[SDL_SCANCODE_LEFT]) {
            my_world.player_pos.x -= right_x * move_speed;
            my_world.player_pos.z -= right_z * move_speed;
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            my_world.player_pos.x += right_x * move_speed;
            my_world.player_pos.z += right_z * move_speed;
        }
        if (state[SDL_SCANCODE_SPACE]) my_world.player_pos.y += move_speed; 
        if (state[SDL_SCANCODE_LSHIFT])my_world.player_pos.y -= move_speed; 
        if (state[SDL_SCANCODE_N]) my_world.rotation.y -= 0.03f; 
        if (state[SDL_SCANCODE_D]) my_world.rotation.y += 0.03f; 
        if (state[SDL_SCANCODE_W]) my_world.rotation.x -= 0.03f; 
        if (state[SDL_SCANCODE_S]) my_world.rotation.x += 0.03f; 

        // Appel de la fonction de dessin avec la lumière dynamique
        drawMesh(original_image, z_buffer, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, my_world, mesh, shading_mode, enable_floor, fov_degrees, normalized_light);
        
        // Appel de la fonction d'anti-aliasing
        if (enable_fxaa) apply_fast_aa(original_image, ORIGINAL_WIDTH, ORIGINAL_HEIGHT);

        // On calcule le frame-rate pour l'afficher
        frame_count++;
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_fps_time >= 1000) {
            current_fps = frame_count;
            frame_count = 0;
            last_fps_time = current_time;
        }

        // Ici on affiche les textes visibles sur l'écran
        draw_string(original_image, ORIGINAL_WIDTH, std::to_string(current_fps), ORIGINAL_WIDTH - 40, 10, 0xFF00FF00);

        uint32_t color_on = 0xFF00FF00;
        uint32_t color_off = 0xFFFF0000;
        uint32_t color_text = 0xFFFFFFFF;

        draw_string(original_image, ORIGINAL_WIDTH, "G:", 10, 10, color_text);
        draw_string(original_image, ORIGINAL_WIDTH, std::to_string(shading_mode), 34, 10, (shading_mode > 0) ? color_on : color_off);

        draw_string(original_image, ORIGINAL_WIDTH, "A:", 10, 24, color_text);
        draw_string(original_image, ORIGINAL_WIDTH, enable_fxaa ? "ON" : "OFF", 34, 24, enable_fxaa ? color_on : color_off);

        draw_string(original_image, ORIGINAL_WIDTH, "S:", 10, 38, color_text);
        draw_string(original_image, ORIGINAL_WIDTH, enable_floor ? "ON" : "OFF", 34, 38, enable_floor ? color_on : color_off);

        draw_string(original_image, ORIGINAL_WIDTH, "FOV:", 10, 52, color_text);
        draw_string(original_image, ORIGINAL_WIDTH, std::to_string((int)fov_degrees), 48, 52, color_on);

        if (current_width != ORIGINAL_WIDTH || current_height != ORIGINAL_HEIGHT) {
            resizing_image(original_image, ORIGINAL_WIDTH, ORIGINAL_HEIGHT, display_image, current_width, current_height);
        } else {
            display_image = original_image;
        }

        SDL_UpdateTexture(texture, nullptr, display_image.data(), current_width * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr); 
        SDL_RenderPresent(renderer); 
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}