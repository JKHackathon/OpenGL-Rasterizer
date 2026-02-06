#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <fstream>
#include <sstream>

#include "ObjParser.hpp"
#include "rasterizer.hpp"

// Compiles shaders from source file, returns false on failure
bool compileShader(GLuint& shader, GLuint program, GLenum shaderType,
                   const char* source) {
    std::ifstream shaderFile;
    shaderFile.open(source);
    if (!shaderFile.is_open()) {
        fprintf(stderr, "Failed to open shader file: %s", source);
        return false;
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    std::string shaderString = shaderStream.str();
    const GLchar* shaderCode = shaderString.c_str();
    shaderFile.close();

    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char shaderLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, shaderLog);
        fprintf(stderr, "Shader compilation error:\n%s\n", shaderLog);
        return false;
    }

    glAttachShader(program, shader);
    return true;
}

bool setVertexShaderInput(Rasterizer& rasterizer) {
    GLuint pos = glGetAttribLocation(rasterizer.curr_state.boundProgram, "pos");
    if (pos == -1) {
        fprintf(stderr, "ERROR: pos not found or optimized out\n");
        return false;
    }
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    return true;
}

int main(int argc, char** argv) {
    GLFWwindow* window;

    // Initialize
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // REQUIRED on macOS
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    window = glfwCreateWindow(640, 480, "Rasterizer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "GLFW context initialization failed\n");
        return -1;
    }
    glfwMakeContextCurrent(window);

    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        fprintf(stderr, "Glew failed to initialize: %s\n",
                glewGetErrorString(glewErr));
    }
    fprintf(stdout, "Using GLEW %s\n", glewGetString(GLEW_VERSION));
    fprintf(stdout, "OpenGL version: %s\n", glGetString(GL_VERSION));

    Rasterizer rasterizer;
    // GLuint vao;
    // glGenVertexArrays(1, &vao);
    // rasterizer.bindVAO(vao);

    ObjParser objData;
    try {
        objData.parse_obj_file("../teapot.obj");
    } catch (std::runtime_error e) {
        fprintf(stderr, "Failed to parse obj file: %s", e.what());
        return -1;
    }

    Mesh mesh(objData);
    fprintf(stdout,
            "objData:\n\tvertices: %lu\n\ttextures: %lu\n\tnormals: "
            "%lu\n\tfaces: %lu\n",
            objData.vertices.size(), objData.vertex_textures.size(),
            objData.vertex_normals.size(), objData.faces.size());
    fprintf(stdout,
            "mesh:\n\tvertices: %lu\n\ttextures: %lu\n\tnormals: "
            "%lu\n\ttriangles: %lu\n",
            mesh.positions.size(), mesh.texcoords.size(), mesh.normals.size(),
            mesh.triangles.size());

    fprintf(stdout, "\n\tv0: %f %f %f\n\tv1: %f %f %f\n\n", mesh.positions[0].x,
            mesh.positions[0].y, mesh.positions[0].z, mesh.positions[1].x,
            mesh.positions[1].y, mesh.positions[1].z);

    // GLuint vbo;
    // glGenBuffers(1, &vbo);
    // rasterizer.bindArrayBuffer(vbo);
    // glBufferData(GL_ARRAY_BUFFER);

    GLuint program = glCreateProgram();

    // Compile vertex shader
    GLuint vs;
    if (!compileShader(vs, program, GL_VERTEX_SHADER, "../shader.vert")) {
        glfwTerminate();
        return -1;
    }

    // Compile fragment shader
    GLuint fs;
    if (!compileShader(fs, program, GL_FRAGMENT_SHADER, "../shader.frag")) {
        glfwTerminate();
        return -1;
    }

    // TODO: maybe move into rasterizer?
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        fprintf(stderr, "Program link error:\n%s\n", log);
        glfwTerminate();
        return -1;
    }

    try {
        rasterizer.bindProgram(program);
    } catch (std::runtime_error e) {
        fprintf(stderr, "Program link error:\n%s\n", e.what());
        glfwTerminate();
        return -1;
    }

    rasterizer.uploadMesh(
        mesh);  // Here because shaders need to be compiled first

    // Inputs, CURRENTLY HANDLED IN UPLOADMESH
    // if (!setVertexShaderInput(rasterizer)) {
    //     glfwTerminate();
    //     return -1;
    // }

    // Display loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, mesh.triangles.size() * 3, GL_UNSIGNED_INT, 0);
        // glDrawArrays(GL_TRIANGLES, 0, mesh.triangles.size() * 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}