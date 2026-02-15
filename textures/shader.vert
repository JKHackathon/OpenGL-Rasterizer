#version 410 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 texcoord;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat3 normal_matrix;

out vec4 view_pos;
out vec3 view_normal;
out vec2 txc;

void main()
{
    gl_Position = mvp * vec4(pos,1);
    view_pos = mv * vec4(pos,1);
    view_normal = normalize(normal_matrix * norm);
    txc = texcoord;//normalize(texcoord); // TODO: does this need to be transformed?
}