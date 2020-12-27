#version 330 core

uniform float intensity;
uniform sampler2D texture_sampler;

in vec3 frag_color;
in vec2 frag_texture_coordinates;

out vec4 color;

void main()
{
	//vec4 rainbow = vec4(frag_color, 1.0f);
	//color = rainbow * texture(texture_sampler, frag_texture_coordinates);
	color = texture(texture_sampler, frag_texture_coordinates);
}
