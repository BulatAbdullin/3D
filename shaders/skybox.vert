#version 330 core

layout (location = 0) in vec3 position;

out vec3 direction;

uniform mat4 projection;
uniform mat4 view;


void main()
{
    /* remove translation from view matrix */
    gl_Position = projection * mat4(mat3(view)) * vec4(position, 1.0);
    gl_Position.z = gl_Position.w; /* skybox must always be in the background */
    direction = position;
}
