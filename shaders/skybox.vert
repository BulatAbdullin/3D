#version 330 core

layout (location = 0) in vec3 position;

out vec3 direction;

uniform mat4 projection;
uniform mat4 view;


void main()
{
    /* remove translation from view matrix */
    gl_Position = projection * mat4(mat3(view)) * vec4(position, 1.0);

    /* skybox must always be in the background */
    gl_Position.z = gl_Position.w;
    /* after the perspective division the z component will be equal to 1.0
    *  so it will be visible only if the are no objects in front of it */

    direction = position;
}
