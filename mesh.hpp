#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <tuple>

#include "ObjParser.hpp"
const double MIN_DOUBLE = std::numeric_limits<double>::lowest();
const double MAX_DOUBLE = std::numeric_limits<double>::max();

struct TupleHash {
    size_t operator()(const std::tuple<int, int, int>& t) const {
        auto [a, b, c] = t;
        size_t h1 = std::hash<int>{}(a);
        size_t h2 = std::hash<int>{}(b);
        size_t h3 = std::hash<int>{}(c);

        // simple but effective combine
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
    // glm::vec3 center;

    BoundingBox() {
        min = glm::vec3(MAX_DOUBLE, MAX_DOUBLE, MAX_DOUBLE);
        max = glm::vec3(MIN_DOUBLE, MIN_DOUBLE, MIN_DOUBLE);
    }

    void add_point(glm::vec3 p) {
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        min.z = std::min(min.z, p.z);
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        max.z = std::max(max.z, p.z);
    }

    glm::vec3 center() {
        return glm::vec3((min.x + max.x) / 2, (min.y + max.y) / 2,
                         (min.z + max.z) / 2);
    }
};

struct Mesh {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> texcoords;

    BoundingBox bounds;

    struct Triangle {
        glm::ivec3 vertices;
    };
    std::vector<Triangle> triangles;

    Mesh(ObjParser& obj) {
        std::unordered_map<std::tuple<int, int, int>, size_t, TupleHash>
            unique_vertices;

        // TODO: separate it out such that each unique vertex has one index
        // each (p,n,t) tuple has its own index
        size_t curr_index = 0;
        for (auto face : obj.faces) {
            glm::ivec3 triangle_verts;
            for (int i = 0; i < 3; i++) {
                int vi = face.vertex_indices[i];
                assert(vi >= 0 && vi < obj.vertices.size());
                
                auto vertex = std::make_tuple(face.vertex_indices[i],
                                              face.vertex_texture_indices[i],
                                              face.vertex_normal_indices[i]);
                auto existing_index = unique_vertices.find(vertex);
                if (existing_index != unique_vertices.end()) {
                    triangle_verts[i] = existing_index->second;
                } else {
                    unique_vertices.emplace(vertex, curr_index);
                    glm::vec3 position = obj.vertices[face.vertex_indices[i]];
                    positions.push_back(position);
                    assert(positions.size() == curr_index + 1);
                    bounds.add_point(position);

                    // TODO: normals/texcoords optional
                    texcoords.push_back(
                        obj.vertex_textures[face.vertex_texture_indices[i]]);
                    assert(texcoords.size() == curr_index + 1);
                    normals.push_back(
                        obj.vertex_normals[face.vertex_normal_indices[i]]);
                    assert(normals.size() == curr_index + 1);
                    triangle_verts[i] = curr_index;
                    curr_index++;
                }
            }
            triangles.emplace_back(triangle_verts);
        }
    }

    glm::mat4 center_mesh_transform() {
        float max_bounds_diff = std::max(
            bounds.max.x - bounds.min.x,
            std::max(bounds.max.y - bounds.min.y, bounds.max.z - bounds.min.z));
        float scale_factor = 2 / max_bounds_diff;

        glm::vec3 center = bounds.center();
        fprintf(stdout,
                "mesh bounds:\n\tmax: %f %f %f\n\tmin: %f %f %f\n\tcenter: %f "
                "%f %f\n\tmax difference: %f\n\n",
                bounds.max.x, bounds.max.y, bounds.max.z, bounds.min.x,
                bounds.min.y, bounds.min.z, center.x, center.y, center.z,
                max_bounds_diff);

        auto transform = glm::mat4(1.0f);
        transform = glm::scale(transform, glm::vec3(scale_factor));
        transform = glm::translate(transform, -center);

        glm::vec4 c = transform * glm::vec4(center, 1.0f);
        printf("transformed center: %f %f %f\n", c.x, c.y, c.z);

        return transform;
    }
};