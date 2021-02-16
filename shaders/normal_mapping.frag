#version 330 core

in vec3 frag_position;
in vec2 frag_texture_coords;

in vs_tangent_space {
    vec3 frag_position;
    vec3 light_position;
    vec3 view_position;
} tangent_space;


out vec4 color;


uniform struct Material
{
    float shininess;
} material;

uniform struct Light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light;

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;


void main()
{
    // normal in tangent space coordinates
    vec3 frag_normal = texture(normal_map, frag_texture_coords).rgb;
    // frag_normal.y = 1.0 - frag_normal.y;
    frag_normal = normalize(frag_normal * 2.0 - 1.0);

    // ambient
    vec3 ambient = light.ambient * vec3(texture(diffuse_map, frag_texture_coords));

    // diffuse
    vec3 light_direction = normalize(tangent_space.light_position - tangent_space.frag_position);
    vec3 diffuse = light.diffuse
            * vec3(texture(diffuse_map, frag_texture_coords))
            * max(dot(frag_normal, light_direction), 0.0f);

    // specular
    vec3 view_direction = normalize(tangent_space.view_position - tangent_space.frag_position);
    vec3 reflect_direction = reflect(-light_direction, frag_normal);
    float specular_strength = max(dot(view_direction, reflect_direction), 0.0f);
    vec3 specular = light.specular 
            * vec3(texture(diffuse_map, frag_texture_coords))
            * pow(specular_strength, material.shininess);

    color = vec4(ambient + diffuse + specular, 1.0f);
}
