#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <fstream>
#include <sstream>

#include "ObjParser.hpp"
#include "rasterizer.hpp"
#include "orbit_camera.hpp"

// NOTE: any struct containing glm types need to be manually aligned or
// allocated as a unique ptr Using alignas should work with smaller types (vec3,
// etc) but not consistently with mat4 Seems to be complicated why this is an
// issue as it is dependent on compiler (more issues with ARM64)
struct AppState {
    OrbitCamera camera;
    glm::mat4 model_matrix;
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    GLint mvp_location;
    double prev_x;
    double prev_y;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mods) {
    // action: press, repeat, release
    // key rollover = limit to # keys detected that are simultaneously pressed
    // (keyboard limit)

    // key: key token (GLFW token)
    // scancode: unique to each physical key (not character), platform-specific
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // TOOD: these are dependent on world and not camera, so upon rotation,
    // changes direction of movement

    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        state->camera.pan(.1,0);
        state->view_matrix = state->camera.calcViewMatrix();
        glm::mat4 mvp =
            state->projection_matrix * state->view_matrix * state->model_matrix;
        glUniformMatrix4fv(state->mvp_location, 1, GL_FALSE, &mvp[0][0]);
    }

    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        state->camera.pan(-.1,0);
        state->view_matrix = state->camera.calcViewMatrix();
        glm::mat4 mvp =
            state->projection_matrix * state->view_matrix * state->model_matrix;
        glUniformMatrix4fv(state->mvp_location, 1, GL_FALSE, &mvp[0][0]);
    }

    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        state->camera.pan(0,.1);
        state->view_matrix = state->camera.calcViewMatrix();
        glm::mat4 mvp =
            state->projection_matrix * state->view_matrix * state->model_matrix;
        glUniformMatrix4fv(state->mvp_location, 1, GL_FALSE, &mvp[0][0]);
    }

    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        state->camera.pan(0,-.1);
        state->view_matrix = state->camera.calcViewMatrix();
        glm::mat4 mvp =
            state->projection_matrix * state->view_matrix * state->model_matrix;
        glUniformMatrix4fv(state->mvp_location, 1, GL_FALSE, &mvp[0][0]);
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos,
                                     double ypos) {
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        state->prev_x = xpos;
        state->prev_y = ypos;
        return;
    }

    // Flipped because rotation around x-axis, not in x direction
    double dy = xpos - state->prev_x;
    double dx = state->prev_y - ypos;

    fprintf(stdout,
            "dx: %f, curr_x: %f, prev_x: %f, dy: %f, curr_y %f, prev_y: %f\n",
            dx, xpos, state->prev_x, dy, ypos, state->prev_y);

    // Rotating around world axes (z is up for some reason, prob
    // defined in obj file) not camera axes 
    // state->view_matrix = glm::rotate(
    //     state->view_matrix, glm::radians(float(dx*.5)), glm::vec3(1, 0, 0));
    // state->view_matrix = glm::rotate(
    //     state->view_matrix, glm::radians(float(dy*.5)), glm::vec3(0, 0, 1));

    state->camera.orbit(dx * .5, dy * .5);
    state->camera.updateBasis();
    state->view_matrix = state->camera.calcViewMatrix();

    glm::mat4 mvp =
        state->projection_matrix * state->view_matrix * state->model_matrix;
    glUniformMatrix4fv(state->mvp_location, 1, GL_FALSE, &mvp[0][0]);

    state->prev_x = xpos;
    state->prev_y = ypos;
}

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
    // AppState* appState = new AppState();
    auto appState_u = std::make_unique<AppState>();
    AppState* appState = appState_u.get();

    glfwSetWindowUserPointer(window, appState);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

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

    appState->mvp_location = glGetUniformLocation(program, "mvp");
    if (appState->mvp_location == -1) {
        fprintf(stderr, "ERROR: mvp uniform not found or optimized out\n");
    }

    // auto transform = glm::scale(glm::mat4(1.0f), glm::vec3(.05,.05,.05));

    appState->model_matrix = mesh.center_mesh_transform();
    // model_matrix = glm::rotate(glm::mat4(1.0f), 1.f, glm::vec3(1, 0, 0)) *
    //                mesh.center_mesh_transform();
    // model_matrix = glm::rotate(model_matrix, glm::radians(-90.f),
    // rotationAxis);

    appState->view_matrix = appState->camera.calcViewMatrix();
        // glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    appState->projection_matrix =
        glm::perspective<float>(glm::radians(60.f), 640.f / 480, 0.1f, 100.f);
    // glm::mat4 orthographic_projection
    glm::mat4 mvp = appState->projection_matrix * appState->view_matrix *
                    appState->model_matrix;

    // glm::vec4 c = mvp * glm::vec4(mesh.bounds.center(), 1.0f);
    // printf("mvp center: %f %f %f\n", c.x, c.y, c.z);

    // TODO: understand the full pipeline!!!
    // Object -> world -> view/camera -> Clip space (clipping coordinate
    // system/homogeneous clip space)
    // -> NDCS (normalized device coordinate system, canonical view volume is
    // bounds of it)
    // -> DCS/screen space (device coordinate system)
    glUniformMatrix4fv(appState->mvp_location, 1, GL_FALSE, &mvp[0][0]);

    // Display loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glDrawElements(GL_TRIANGLES, mesh.triangles.size() * 3, GL_UNSIGNED_INT,
        //                0);
        glDrawArrays(GL_TRIANGLES, 0, mesh.triangles.size() * 3);
        // glm::vec3 rotationAxis(1, 0, 0);
        // appState.view_matrix = glm::rotate(appState.view_matrix,
        // glm::radians(1.f), glm::vec3(1, 0, 0)); glm::mat4 mvp =
        // perspective_projection * appState.view_matrix * model_matrix;
        // glUniformMatrix4fv(location, 1, GL_FALSE, &mvp[0][0]);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}