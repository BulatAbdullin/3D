#version 330 core

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texture_coords;

out vec4 color;

uniform vec3 view_position;
uniform Material material;
uniform Light light;

void main()
{
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, frag_texture_coords));
    
    // diffuse
    vec3 light_direction = normalize(light.position - frag_position);
    vec3 diffuse = light.diffuse
            * vec3(texture(material.texture_diffuse1, frag_texture_coords))
            * max(dot(frag_normal, light_direction), 0.0f);

    // specular
    vec3 view_direction = normalize(view_position - frag_position);
    vec3 reflect_direction = reflect(-light_direction, frag_normal);
    float specular_strength = max(dot(view_direction, reflect_direction), 0.0f);
    vec3 specular = light.specular 
            * vec3(texture(material.texture_specular1, frag_texture_coords))
            * pow(specular_strength, material.shininess);

    color = vec4(ambient + diffuse + specular, 1.0f);
}
