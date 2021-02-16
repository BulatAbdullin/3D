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
uniform sampler2D displacement_map;
uniform sampler2D specular_map;


vec4 phong(vec3 normal, vec2 texture_coords);

void main()
{
    /* Parallax Mapping */

    vec3 view_direction = normalize(tangent_space.view_position - tangent_space.frag_position);

    float num_layers = 32;
    float h = 1.0 / num_layers; /* layer's depth */

    int i = 0;
    float depth = texture(displacement_map, frag_texture_coords).r;
    for (float layer_depth = 0.0; layer_depth < depth; layer_depth += h, i++) {
        depth = texture(displacement_map, frag_texture_coords).r;
    }

    const float depth_scale = 0.1;
    vec2 offset = depth_scale * view_direction.xy * (i * h);
    vec2 displaced_texture_coords = frag_texture_coords - offset;
    if (displaced_texture_coords.x > 1.0 || displaced_texture_coords.y > 1.0
            || displaced_texture_coords.x < 0.0 || displaced_texture_coords.y < 0.0) {
        discard;
    }


    /* Parallax Occlusion Mapping (linear interpolation) */


    /* Normal Mapping */

    // normal in tangent space coordinates
    vec3 frag_normal = texture(normal_map, displaced_texture_coords).rgb;
    // frag_normal.y = 1.0 - frag_normal.y;
    frag_normal = normalize(frag_normal * 2.0 - 1.0);
    
    color = phong(frag_normal, displaced_texture_coords);
}


vec4 phong(vec3 normal, vec2 texture_coords)
{
    // ambient
    vec3 ambient = light.ambient * vec3(texture(diffuse_map, texture_coords));

    // diffuse
    vec3 light_direction = normalize(tangent_space.light_position - tangent_space.frag_position);
    vec3 diffuse = light.diffuse
            * vec3(texture(diffuse_map, texture_coords))
            * max(dot(normal, light_direction), 0.0f);

    // specular
    vec3 view_direction = normalize(tangent_space.view_position - tangent_space.frag_position);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float specular_strength = max(dot(view_direction, reflect_direction), 0.0f);
    vec3 specular = light.specular 
            * vec3(texture(specular_map, texture_coords))
            * pow(specular_strength, material.shininess);


    return vec4(ambient + diffuse + specular, 1.0f);
}
