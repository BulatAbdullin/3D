#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

out vec2 frag_texture_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    frag_texture_coords = texture_coords;

    gl_Position = projection * view * model * vec4(vertex_position, 1.0f);
}
