#ifndef IMAGE_H
#define IMAGE_H

#include <cmath>
#include <vector>
#include <sys/types.h>
#include <cstdint>
#include <iostream>
using namespace std;

// Resize the image à partir d'un algorithme des plus proches voisins -> pixellise l'image
void resizing_image_nearest_neighbour(std::vector<u_int32_t>& original_image,
                    int original_width,
                    int original_height,
                    std::vector<u_int32_t>& target_image,
                    int target_width,
                    int target_height);

// Resize the image à partir d'une interpolation selon x et y -> floute l'image
void resizing_image_bilinear_interpolation(std::vector<u_int32_t>& original_image,
                    int original_width,
                    int original_height,
                    std::vector<u_int32_t>& target_image,
                    int target_width,
                    int target_height);


#endif