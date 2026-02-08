#version 410 core

layout(location=0) out vec4 color;

in vec4 normal;

void main() {
    normal.x == clamp(normal.x, 0, 1);
    normal.y == clamp(normal.y, 0, 1);
    normal.z == clamp(normal.z, 0, 1);
    color = normal;
}