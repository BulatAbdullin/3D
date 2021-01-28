#ifndef MODEL_HPP
#define MODEL_HPP

#include "mesh.hpp"
#include "shader_program.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
private:
    Mesh *meshes;
    unsigned int num_meshes;
    unsigned int current_mesh;

    const char *directory;

    inline void add_mesh(const Mesh& mesh)
    {
        meshes[current_mesh++] = mesh;
    }

    void process_node(const aiNode *node, const aiScene *scene);
    Mesh process_mesh(const aiMesh *mesh, const aiScene *scene);
    Vertex *process_vertices(const aiMesh *mesh, unsigned int& num_vertices);
    GLuint *process_indices(const aiMesh *mesh, unsigned int& num_indices);
    Texture *process_textures(const aiMesh *mesh, const aiScene *scene,
                              unsigned int& num_textures);
    GLuint texture_from_file(const char *filename, const char *directory);

public:
    Model(const char *filepath);
    ~Model();
    void draw(const ShaderProgram& shader_program) const;
};

#endif /* ifndef MODEL_HPP */
