#include "../include/file_opening.hpp"


bool parseOBJ(const std::string& filename, std::vector<float3>& outVertices, std::vector<Face>& outFaces) {
    std::ifstream file(filename);
    
    // Essaye d'ouvrir le fichier si erreur -> renvoie False
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    // On parse chaque ligne du fichier
    std::string line;
    while (std::getline(file, line)) {
        
        if (line.empty()) continue;

        // Traitemetn de la string
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        // Parse les sommets
        if (prefix == "v") {
            float3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            outVertices.push_back(vertex);
        }
        // Parse les faces
        else if (prefix == "f") {
            Face face;
            iss >> face.v1 >> face.v2 >> face.v3;
            outFaces.push_back(face);
        }
    }

    file.close();
    return true;
}
