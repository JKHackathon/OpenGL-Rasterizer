#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

const glm::vec3 WORLD_UP(0, 1, 0);

struct OrbitCamera {
    glm::vec3 target = glm::vec3(0);
    float radius; // allows for dolly (movement towards target)
    float pitch;  // up/down, around x-axis
    float yaw;    // right/left, around y-axis

    glm::vec3 pos;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;

    OrbitCamera()
        : pos(glm::vec3(0, 0, 2)),
          forward(glm::vec3(0, 0, -1)),
          right(glm::vec3(1, 0, 0)),
          up(WORLD_UP),
          yaw(-90),
          radius(2) {}

    void updateBasis() {
        // TODO: switch to quaternions to prevent gimbal lock/flip, also makes rotation simpler
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        forward.x = std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw));
        forward.y = std::sin(glm::radians(pitch));
        forward.z = std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw));
        forward = glm::normalize(forward);

        pos = target - forward * radius;

        right = glm::normalize(glm::cross(WORLD_UP, forward));
        up = glm::normalize(glm::cross(forward, right));
    }

    void orbit(float delta_pitch, float delta_yaw) {
        pitch += delta_pitch;
        yaw += delta_yaw;
        updateBasis();
    }

    void pan(float dx, float dy) {
        float panSpeed = radius * .1; // speed dependent on distance from object
        target += (-right * dx + up * dy) * panSpeed;
        updateBasis();
    }

    // dolly

    glm::mat4 calcViewMatrix() { return glm::lookAt(pos, target, up); }
};