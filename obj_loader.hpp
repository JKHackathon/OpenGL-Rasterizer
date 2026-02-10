#pragma once

#include <charconv>
#include <filesystem>
#include <format>
#include <fstream>
#include <glm/vec3.hpp>
#include <vector>

#include "materials.hpp"

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
    void parse_mtl_file(std::string mtl_filename);
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> vertex_normals;
    std::vector<glm::vec3> vertex_textures;

    std::vector<Face> faces;
    // TODO: should i be creating the material pointers here?
    std::unordered_map<std::string, std::unique_ptr<Material>> materials;

   private:
    std::tuple<int, std::optional<int>, std::optional<int>>
    get_face_vertex_index(std::string_view face_index_group);
};