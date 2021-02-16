#version 330 core

in vec2 frag_texture_coords;

out vec4 color;

uniform sampler2D texture1;

void main()
{             
    color = texture(texture1, frag_texture_coords);
} 
