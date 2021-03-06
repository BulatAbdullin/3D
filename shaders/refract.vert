#version 330 core 

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 frag_position;
out vec3 frag_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    frag_position = vec3(model * vec4(position, 1.0));
    frag_normal = transpose(inverse(mat3(model))) * normal;
    frag_normal = normalize(frag_normal);
    gl_Position = projection * view * model * vec4(position, 1.0);
}
