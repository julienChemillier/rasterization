#include "../include/image.hpp"

void resizing_image_nearest_neighbour(std::vector<u_int32_t>& original_image,
                                      int original_width,
                                      int original_height,
                                      std::vector<u_int32_t>& target_image,
                                      int target_width,
                                      int target_height) 
{
    // Pour éviter les divisions par 0
    if (target_width == 0 || target_height == 0) return;

    for (int y = 0; y < target_height; ++y) {
        for (int x = 0; x < target_width; ++x) {
            
            // On calcule quel pixel de l'image originale donne le pixel actuel (de la nouvelle image)
            int src_x = (x * (original_width-1)) / target_width;
            int src_y = (y * (original_height-1)) / target_height;

            // On applique les changements
            target_image[y * target_width + x] = original_image[src_y * original_width + src_x];
        }
    }
}

void resizing_image_bilinear_interpolation(std::vector<u_int32_t>& original_image,
                                          int original_width,
                                          int original_height,
                                          std::vector<u_int32_t>& target_image,
                                          int target_width,
                                          int target_height) 
{
    // Pour éviter les divisions par 0
    if (target_width == 0 || target_height == 0) return;

    float x_ratio = (float)original_width / target_width;
    float y_ratio = (float)original_height / target_height;
    
    // On parallélise brutalement les calcules
    #pragma omp parallel for
    for (int y = 0; y < target_height; ++y) {
        for (int x = 0; x < target_width; ++x) {
            
            float gx = x * x_ratio;
            float gy = y * y_ratio;
            
            int x1 = (int)gx;
            int y1 = (int)gy;
            int x2 = x1 + 1;
            int y2 = y1 + 1;

            if (x2 >= original_width) x2 = original_width - 1;
            if (y2 >= original_height) y2 = original_height - 1;

            float dx = gx - x1;
            float dy = gy - y1;

            // On récupère les valeurs des 4 pixels les plus proches
            uint32_t p1 = original_image[y1 * original_width + x1];
            uint32_t p2 = original_image[y1 * original_width + x2];
            uint32_t p3 = original_image[y2 * original_width + x1];
            uint32_t p4 = original_image[y2 * original_width + x2];

            // On précalcule les poids pour l'interpolation bilinéaire
            float w1 = (1 - dx) * (1 - dy);
            float w2 = dx * (1 - dy);
            float w3 = (1 - dx) * dy;
            float w4 = dx * dy;

            // On extrait la couleur rouge et l'interpole (bits 17-24)
            float r = ((p1 >> 16) & 0xFF) * w1 +
                      ((p2 >> 16) & 0xFF) * w2 +
                      ((p3 >> 16) & 0xFF) * w3 +
                      ((p4 >> 16) & 0xFF) * w4;

            // On extrait la couleur verte et l'interpole (bits 9-16)
            float g = ((p1 >> 8) & 0xFF) * w1 +
                      ((p2 >> 8) & 0xFF) * w2 +
                      ((p3 >> 8) & 0xFF) * w3 +
                      ((p4 >> 8) & 0xFF) * w4;

            // On extrait la couleur bleue et l'interpole (bits 1-8)
            float b = (p1 & 0xFF) * w1 +
                      (p2 & 0xFF) * w2 +
                      (p3 & 0xFF) * w3 +
                      (p4 & 0xFF) * w4;

            // Arrondis à l'entier le plus proche
            uint32_t final_r = (uint32_t)(r + 0.5f);
            uint32_t final_g = (uint32_t)(g + 0.5f);
            uint32_t final_b = (uint32_t)(b + 0.5f);

            // Combine les 3 couleurs pour créer le pixel (alpha, rouge, vert, bleu)
            uint32_t final_pixel = (255 << 24) | (final_r << 16) | (final_g << 8) | final_b;

            // Applique les changements sur l'image
            target_image[y * target_width + x] = final_pixel;
        }
    }
}