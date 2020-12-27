#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 vertex_texture_coordinates;

out vec2 frag_texture_coordinates;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat4 transform = projection * view * model;
	gl_Position = transform * vec4(vertex_position, 1.0f);
	frag_texture_coordinates = vertex_texture_coordinates;
}
