#pragma once

#include <charconv>
#include <fstream>
#include <glm/vec3.hpp>
#include <vector>

struct Face {
    glm::ivec3 vertex_indices;
    glm::ivec3 vertex_texture_indices;
    glm::ivec3 vertex_normal_indices;
};

struct ObjParser {
   public:
    // TODO: this currently converts indices to 0-based (can change this to remain 1-based and rely on mesh to convert)
    void parse_obj_file(const char* filename);
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> vertex_normals;
    std::vector<glm::vec3> vertex_textures;

    std::vector<Face> faces;

   private:
    std::tuple<int, std::optional<int>, std::optional<int>>
    get_face_vertex_index(std::string_view face_index_group);
};