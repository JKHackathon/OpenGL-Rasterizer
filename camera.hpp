#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

const glm::vec3 WORLD_UP(0, 1, 0);

struct Camera {
    glm::vec3 pos;
    float pitch;  // up/down, around x-axis
    float yaw;    // right/left, around y-axis

    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;

    Camera()
        : pos(glm::vec3(0, 0, 5)),
          forward(glm::vec3(0, 0, -1)),
          right(glm::vec3(1, 0, 0)),
          up(WORLD_UP),
          yaw(-90) {}

    // First-person camera rotation
    void updateBasis(float delta_pitch, float delta_yaw) {
        // TODO: x, z axis might be flipped
        pitch += delta_pitch;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        yaw += delta_yaw;
        forward.x = std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw));
        forward.y = std::sin(glm::radians(pitch));
        forward.z = std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw));
        forward = glm::normalize(forward);

        right = glm::normalize(glm::cross(WORLD_UP, forward));
        up = glm::normalize(glm::cross(forward, right));
    }

    // Target-based orbit camera rotation around world origin
    void updateBasis() {
        forward = glm::normalize(-pos);  // look at origin
        right = glm::normalize(glm::cross(forward, WORLD_UP));
        up = glm::normalize(glm::cross(right, forward));
    }

    void rotateAroundOrigin(float dx, float dy) {
        glm::mat4 rot(1.0f);

        rot = glm::rotate(rot, glm::radians(dx * 0.5f), glm::vec3(1, 0, 0));
        rot = glm::rotate(rot, glm::radians(dy * 0.5f), glm::vec3(0, 1, 0));

        pos = glm::vec3(rot * glm::vec4(pos, 1.0f));
    }

    glm::mat4 calcViewMatrix() { return glm::lookAt(pos, pos + forward, up); }
};