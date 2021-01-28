#ifndef MESH_HPP
#define MESH_HPP

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "shader_program.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture_coords;
};

enum TextureType
{
    DIFFUSE, SPECULAR
};

struct Texture
{
    GLuint id;
    enum TextureType type;
};


class Mesh
{
private:
    GLuint VAO; /* Vertex Array Object */
    GLuint VBO; /* Vertex Buffer Object */
    GLuint EBO; /* Element Buffer Object */

    int num_vertices;
    int num_indices;

    Texture *textures;
    int num_textures;

    void setup_attrib_pointers() const;
public:
    Mesh(const Vertex *vertices, int num_vertices,
         const GLuint *indices, int num_indices,
         const Texture *textures, int num_textures);

    Mesh()
    {}

    void draw(const ShaderProgram& shader_program) const;
};

#endif /* ifndef MESH_HPP */
