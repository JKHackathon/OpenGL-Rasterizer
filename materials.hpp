#include <glm/vec3.hpp>
#include <string>

struct TextureMap {
    std::vector<unsigned char> pixels;
    unsigned int width;
    unsigned int height;
};

struct Material {
    glm::vec3 K_a;      // ambient
    glm::vec3 K_d;      // diffuse
    glm::vec3 K_s;      // specular
    float shininess;    // Ns
    float transparency; // Tr = 1 - d (dissolve)
    glm::vec3 transmission_color; // TFC, Tf
    float ior;          // Index of refraction (Ni)
    // TODO: do I need this? how is this handled?
    int illum;          // illumination model, often not used

    // Texture Maps
    std::shared_ptr<TextureMap> ambient_map_filepath; // TODO: should these be the data? Yes. I only want to load this in once
    std::shared_ptr<TextureMap> diffuse_map_filepath;
    std::shared_ptr<TextureMap> specular_map_filepath;
    std::shared_ptr<TextureMap> bump_map_filepath;
    // alpha, normal, displacement, etc
};