#version 410 core

layout(location=0) out vec4 color;

in vec4 view_pos;
in vec3 view_normal;

// TODO: need to input material data
// const vec4 K_d = vec4(1,0,0,1);
// const vec4 K_s = vec4(1,1,1,1);
// const vec4 K_a = K_d;
// const float shininess = 60;
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float ior;
    float transparency;
    vec3 transmission_color;
};

uniform Material material = Material(vec3(.6,.6,.6),vec3(.6,.6,.6),vec3(.7,.7,.7), 20, 0,0,vec3(0,0,0));

// Textures
uniform sampler2D tex;
in vec2 txc;

// Lights
const float I = .2;
uniform vec4 view_light_pos;
uniform vec4 view_camera_pos;

vec3 diffuse(vec4 dir_to_light, vec3 N) {
    return vec3(texture(tex, txc)) * I * dot(N, vec3(dir_to_light));
    //return material.diffuse * I * dot(N, vec3(dir_to_light));
}

vec3 ambient() {
    return vec3(texture(tex, txc)) * I;
    // return material.ambient * I;
}

vec3 specular(vec4 dir_to_light, vec3 N) {
    // vec3 reflection = 2 * dot(vec3(dir_to_light), view_normal) * view_normal - vec3(dir_to_light);
    vec4 dir_to_camera = normalize(view_camera_pos - view_pos);
    vec4 H = normalize(dir_to_camera + dir_to_light);
    float cos_phi = max(dot(N,vec3(H)), 0);//max(dot(vec3(dir_to_camera), reflection), 0);
    return material.specular * pow(cos_phi, material.shininess);
}

void main() {
    vec3 N = normalize(view_normal); // Note: when interpolated, no longer normalized
    vec4 dir_to_light = normalize(view_light_pos - view_pos);
    color = vec4(specular(dir_to_light, N) + ambient() + diffuse(dir_to_light, N),1);
}