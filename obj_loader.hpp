#pragma once

#include <charconv>
#include <filesystem>
#include <format>
#include <fstream>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <unordered_set>
#include <vector>

#include "materials.hpp"
#include "external/lodepng.h"

struct Face {
    glm::ivec3 vertex_indices;
    glm::ivec3 vertex_texture_indices;
    glm::ivec3 vertex_normal_indices;
    std::optional<std::string> material;
};

struct ObjLoader {
   public:
    // TODO: this currently converts indices to 0-based (can change this to
    // remain 1-based and rely on mesh to convert)
    void parse_obj_file(const char* filename);
    void parse_mtl_file(std::string filepath_dir, std::string mtl_filename);
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> vertex_normals;
    std::vector<glm::vec2> vertex_textures;

    std::vector<Face> faces;
    // TODO: should i be creating the material pointers here?
    std::unordered_map<std::string, std::unique_ptr<Material>> materials;
    std::unordered_map<std::string, std::shared_ptr<TextureMap>> texture_maps;

    // Loaded materials and texture maps for efficiency
    // TODO: how am i redirecting the data tho? Solution: use a map to shared_ptr
    std::unordered_set<std::string> loaded_materials;
    std::unordered_set<std::string> loaded_texture_maps;

   private:
    std::tuple<int, std::optional<int>, std::optional<int>>
    get_face_vertex_index(std::string_view face_index_group);
    void decode_texture_png(std::string filename, TextureMap* textureMap);
};