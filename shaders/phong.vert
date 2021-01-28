#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texture_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    frag_position = vec3(model * vec4(position, 1.0f));
    frag_normal = normalize(mat3(transpose(inverse(model))) * normal);
    frag_texture_coords = texture_coords;

    gl_Position = projection * view * model * vec4(position, 1.0f);
}
