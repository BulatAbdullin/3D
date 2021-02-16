#version 330 core

in vec3 frag_position;
in vec3 frag_normal;

out vec4 color;

uniform vec3 view_position;
uniform samplerCube cubemap;

void main()
{
    vec3 view_direction = normalize(frag_position - view_position);
    vec3 reflection_direction = reflect(view_direction, frag_normal);
    color = texture(cubemap, reflection_direction);
}
