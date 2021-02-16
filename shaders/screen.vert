#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texture_coords;

out vec2 frag_texture_coords;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0); 
    frag_texture_coords = texture_coords;
} 
