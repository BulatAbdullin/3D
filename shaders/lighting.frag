#version 330 core

out vec4 color;

uniform vec3 object_color;
uniform vec3 light_color;

void main()
{
    float ambient_factor = 0.1;
    color = vec4(ambient_factor * object_color, 1.0);
}
