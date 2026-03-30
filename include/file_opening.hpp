#ifndef FILE_OPENING_H
#define FILE_OPENING_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "points.hpp"

// Structure pour contenir une face d'un mesh
struct Face {
    int v1, v2, v3;
};

// Parse un fichier .obj à partir de son nom, stock les données dans outVerticeset outFaces
// Renvoie True si tout s'est bien passé
bool parseOBJ(const std::string& filename, std::vector<float3>& outVertices, std::vector<Face>& outFaces);

#endif