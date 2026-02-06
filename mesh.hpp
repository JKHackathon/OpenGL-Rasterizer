#include <glm/vec3.hpp>
#include <tuple>

#include "ObjParser.hpp"

struct TupleHash {
    size_t operator()(const std::tuple<int,int,int>& t) const {
        auto [a, b, c] = t;
        size_t h1 = std::hash<int>{}(a);
        size_t h2 = std::hash<int>{}(b);
        size_t h3 = std::hash<int>{}(c);

        // simple but effective combine
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct Mesh {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> texcoords;

    struct Triangle {
        glm::ivec3 vertices;
    };
    std::vector<Triangle> triangles;

    Mesh(ObjParser& obj) {
        std::unordered_map<std::tuple<int, int, int>, size_t, TupleHash> unique_vertices;

        // TODO: separate it out such that each unique vertex has one index
        // each (p,n,t) tuple has its own index
        size_t curr_index = 0;
        for (auto face : obj.faces) {
            glm::ivec3 triangle_verts;
            for (int i = 0; i < 3; i++) {
                auto vertex = std::make_tuple(face.vertex_indices[i],
                                              face.vertex_texture_indices[i],
                                              face.vertex_normal_indices[i]);
                auto existing_index = unique_vertices.find(vertex);
                if (existing_index != unique_vertices.end()) {
                    triangle_verts[i] = existing_index->second;
                } else {
                    unique_vertices.emplace(vertex, curr_index);
                    positions.push_back(obj.vertices[face.vertex_indices[i]]);
                    assert(positions.size() == curr_index + 1);
                    texcoords.push_back(
                        obj.vertices[face.vertex_texture_indices[i]]);
                    assert(texcoords.size() == curr_index + 1);
                    normals.push_back(
                        obj.vertices[face.vertex_normal_indices[i]]);
                    assert(normals.size() == curr_index + 1);
                    triangle_verts[i] = curr_index;
                    curr_index++;
                }
            }
            triangles.emplace_back(triangle_verts);
        }
    }
};