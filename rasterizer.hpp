#pragma once
#include <OpenGL/gl3.h>

#include <cstdio>
#include <stdexcept>

#include "mesh.hpp"

struct GLState {
    GLuint boundProgram;
    GLuint boundVAO;
    GLuint boundArrayBuffer;
    GLuint boundElementBuffer;
};

class Rasterizer {
   public:
    void bindProgram(GLuint program) {
        if (program == curr_state.boundProgram) {
            return;
        }
        GLint linkStatus;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            throw std::runtime_error("Program not yet linked\n");
        }
        glUseProgram(program);
        curr_state.boundProgram = program;
    }

    void bindVAO(GLuint vao) {
        if (curr_state.boundVAO == vao) {
            return;
        }
        glBindVertexArray(vao);
        curr_state.boundVAO = vao;
    }

    // TODO: generally a vbo, but not always
    void bindArrayBuffer(GLuint vbo) {
        if (curr_state.boundArrayBuffer == vbo) {
            return;
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        curr_state.boundArrayBuffer = vbo;
    }

    void bindElementBuffer(GLuint ebo) {
        if (curr_state.boundElementBuffer == ebo) {
            return;
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        curr_state.boundElementBuffer = ebo;
    }

    GLState curr_state;

    struct VertexData {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
    };

    void uploadMesh(Mesh& mesh) {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        bindVAO(vao);
        GLuint vbo;
        glGenBuffers(1, &vbo);
        bindArrayBuffer(vbo);

        // Set up unified vertex buffer
        std::vector<VertexData> vertices;
        vertices.reserve(mesh.positions.size() * 3);
        for (int i = 0; i < mesh.positions.size(); i++) {
            vertices.emplace_back(mesh.positions[i], mesh.normals[i],
                                  glm::normalize(mesh.texcoords[i])); // Change to st-coords
        }

        // For drawing without EBO
        // for (auto& triangle : mesh.triangles) {
        //     vertices.push_back(mesh.positions[triangle.vertices.x]);
        //     vertices.push_back(mesh.positions[triangle.vertices.y]);
        //     vertices.push_back(mesh.positions[triangle.vertices.z]);
        // }
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData),
                     vertices.data(), GL_STATIC_DRAW);

        // Set up element array buffer (indices)
        // std::vector<glm::ivec3> indices;  // NOTE: must be unsigned int!
        std::vector<unsigned int> indices;
        for (auto& triangle : mesh.triangles) {
            indices.push_back(triangle.vertices.x);
            indices.push_back(triangle.vertices.y);
            indices.push_back(triangle.vertices.z);
            // indices.push_back(triangle.vertices);
        }

        GLuint ebo;
        glGenBuffers(1, &ebo);
        bindElementBuffer(ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(unsigned int), indices.data(),
                     GL_STATIC_DRAW);

        // TODO: do i also assign to attributes now? Make sure after shaders
        // compiled position
        GLuint pos = glGetAttribLocation(curr_state.boundProgram, "pos");
        if (pos == -1) {
            fprintf(stderr, "ERROR: pos not found or optimized out\n");
            // return false;
        }
        glEnableVertexAttribArray(pos);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 3,
                              (GLvoid*)0);
        // For without EBO
        // glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        // normal
        GLuint norm = glGetAttribLocation(
            curr_state.boundProgram,
            "norm");  // Note: optimized out by linker if not used
        if (norm == -1) {
            fprintf(stderr, "ERROR: norm not found or optimized out\n");
            // return false;
        }
        glEnableVertexAttribArray(norm);
        glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE,
                              sizeof(glm::vec3) * 3,
                              (GLvoid*)sizeof(glm::vec3));

        // texcoords
        GLuint texcoord =
            glGetAttribLocation(curr_state.boundProgram, "texcoord");
        if (texcoord == -1) {
            fprintf(stderr, "ERROR: texcoord not found or optimized out\n");
            // return false;
        }
        glEnableVertexAttribArray(texcoord);  // pos
        glVertexAttribPointer(texcoord, 3, GL_FLOAT, GL_FALSE,
                              sizeof(glm::vec2) * 3,
                              (GLvoid*)(sizeof(glm::vec3) * 2));

        // TODO: unbind VAO? why? Do i want this to be self-contained? probably.
        // How do i give access to the ids tho?
    }

   private:
};