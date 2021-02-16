#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;
layout (location = 3) in vec3 tangent;

out vec3 frag_position;
out vec2 frag_texture_coords;

out vs_tangent_space {
    vec3 frag_position;
    vec3 light_position;
    vec3 view_position;
} tangent_space;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;
uniform vec3 view_position;


void main()
{
    frag_position = vec3(model * vec4(position, 1.0f));
    frag_texture_coords = texture_coords;

    // a normal matrix is used to translate normal vectors
    mat3 normal_matrix = inverse(transpose(mat3(model)));

    // construct TBN matrix
    vec3 T = normalize(normal_matrix * tangent); // Tangent
    vec3 N = normalize(normal_matrix * normal);  // Normal
    T = normalize(T - dot(T, N) * N); // Gram-Schmidt orthogonalization
    vec3 B = cross(N, T); // Bitangent
    mat3 TBN = mat3(T, B, N); // T, B and N are columns
    mat3 TBN_inverse = transpose(TBN); // since TBN is orthogonal

    // actually the same for a simple plane
    // mat3 TBN_inverse = transpose(mat3(model));


    // transform everything to tangent space coordinates
    tangent_space.frag_position = TBN_inverse * frag_position;
    tangent_space.light_position = TBN_inverse * light_position;
    tangent_space.view_position = TBN_inverse * view_position;

    gl_Position = projection * view * model * vec4(position, 1.0f);
}
