#version 410 core

layout(location=0) out vec4 color;

in vec4 view_pos;
in vec3 view_normal;

const vec4 K_d = vec4(1,0,0,1);
const vec4 K_s = vec4(1,1,1,1);
const vec4 K_a = K_d;
const float I = .2;
const float shininess = 60;

uniform vec4 view_light_pos;
uniform vec4 view_camera_pos;

vec4 diffuse(vec4 dir_to_light, vec3 N) {
    return K_d * I * dot(N, vec3(dir_to_light));
}

vec4 ambient(vec4 dir_to_light) {
    return K_a * I;
}

vec4 specular(vec4 dir_to_light, vec3 N) {
    // vec3 reflection = 2 * dot(vec3(dir_to_light), view_normal) * view_normal - vec3(dir_to_light);
    vec4 dir_to_camera = normalize(view_camera_pos - view_pos);
    vec4 H = normalize(dir_to_camera + dir_to_light);
    float cos_phi = max(dot(N,vec3(H)), 0);//max(dot(vec3(dir_to_camera), reflection), 0);
    return K_s * pow(cos_phi, shininess);
}

void main() {
    vec3 N = normalize(view_normal); // Note: when interpolated, no longer normalized
    vec4 dir_to_light = normalize(view_light_pos - view_pos);
    color = specular(dir_to_light, N) + ambient(dir_to_light) + diffuse(dir_to_light, N);
}