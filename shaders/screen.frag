#version 330 core

in vec2 frag_texture_coords;

out vec4 color;
  
uniform sampler2D screen_texture;

void main()
{ 
    color = texture(screen_texture, frag_texture_coords);
    float weighted = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = vec4(vec3(weighted), 1.0);
}
