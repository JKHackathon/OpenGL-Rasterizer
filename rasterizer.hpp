#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <stdexcept>

#include "mesh.hpp"

struct GLState {
    GLuint boundProgram;
    GLuint boundVAO;
    GLuint boundArrayBuffer;
    GLuint boundElementBuffer;
};

// TODO: this API can be improved immensely
class Rasterizer {
   public:
    GLState curr_state;

    struct VertexData {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
    };

    void bindProgram(GLuint program);
    void bindVAO(GLuint vao);
    // TODO: generally a vbo, but not always
    void bindArrayBuffer(GLuint vbo);
    void bindElementBuffer(GLuint ebo);

    void uploadVec3(const GLchar* varName, glm::vec3 data);
    void uploadFloat(const GLchar* varName, float data);

    void uploadMesh(Mesh& mesh);
    void upload_material(Material* material);
    void upload_texture(TextureMap* texture);

   private:
};