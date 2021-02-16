#include <stdio.h>  /* snprintf */
#include <libgen.h> /* dirname */
#include <limits.h> /* PATH_MAX */
#include <string.h>

#include <SOIL/SOIL.h>

#include "model.hpp"


Model::Model(const char *filepath)
    : current_mesh(0)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        filepath, aiProcess_Triangulate | aiProcess_GenNormals);

    if (scene == NULL
            || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
            || scene->mRootNode == NULL) {
        throw importer.GetErrorString();
    }

    char filepath_copy[PATH_MAX];
    strncpy(filepath_copy, filepath, sizeof(filepath_copy));
    directory = dirname(filepath_copy);

    num_meshes = scene->mNumMeshes;
    meshes = new Mesh[num_meshes];

    process_node(scene->mRootNode, scene);
}


Model::~Model()
{
    delete[] meshes;
}


void Model::process_node(const aiNode *node, const aiScene *scene)
{
    // process the node's meshes
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        add_mesh(process_mesh(mesh, scene));
    }

    // recursively process the node's children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}


Mesh Model::process_mesh(const aiMesh *mesh, const aiScene *scene)
{
    unsigned int num_vertices, num_indices, num_textures;
    Vertex *vertices = process_vertices(mesh, num_vertices);
    GLuint *indices = process_indices(mesh, num_indices);
    Texture *textures = process_textures(mesh, scene, num_textures);

    Mesh new_mesh(vertices, num_vertices, indices, num_indices,
                  textures, num_textures);

    delete[] vertices;
    delete[] indices;
    delete[] textures;

    return new_mesh;
}


Vertex *Model::process_vertices(const aiMesh *mesh, unsigned int& num_vertices)
{
    num_vertices = mesh->mNumVertices;
    Vertex *vertices = new Vertex[num_vertices];
    for (unsigned int i = 0; i < num_vertices; i++) {
        glm::vec3 position;
        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;
        vertices[i].position = position;

        glm::vec3 normal;
        normal.x = mesh->mNormals[i].x;
        normal.y = mesh->mNormals[i].y;
        normal.z = mesh->mNormals[i].z;
        vertices[i].normal = normal;

        glm::vec2 texture_coords(0.0f, 0.0f);
        /* check if the mesh contains texture coordinates */
        if (mesh->mTextureCoords[0]) {
            texture_coords.x = mesh->mTextureCoords[0][i].x;
            texture_coords.y = mesh->mTextureCoords[0][i].y;
        }
        vertices[i].texture_coords = texture_coords;
    }
    return vertices;
}


GLuint *Model::process_indices(const aiMesh *mesh, unsigned int& num_indices)
{
    /* faces are always triangles, i.e. mesh->mFaces[i].mNumIndices == 3 */
    num_indices = (mesh->mNumFaces) * (mesh->mFaces[0].mNumIndices);
    GLuint *indices = new GLuint[num_indices];
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices[3 * i + j] = face.mIndices[j];
        }
    }
    return indices;
}


Texture *Model::process_textures(const aiMesh *mesh, const aiScene *scene,
                                 unsigned int& num_textures)
{
    /* check if the mesh contains a material or not */
    if (mesh->mMaterialIndex < 0) {
        num_textures = 0;
        return NULL;
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    num_textures = material->GetTextureCount(aiTextureType_DIFFUSE)
                 + material->GetTextureCount(aiTextureType_SPECULAR);
    Texture *textures = new Texture[num_textures];
    aiString filename;
    unsigned int i;
    for(i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
        material->GetTexture(aiTextureType_DIFFUSE, i, &filename);
        textures[i].id = this->texture_from_file(filename.C_Str(), this->directory);
        textures[i].type = DIFFUSE;
        //texture.filename = filename; // What's this?
    }
    for(unsigned j = 0; j < material->GetTextureCount(aiTextureType_SPECULAR); j++) {
        material->GetTexture(aiTextureType_DIFFUSE, i, &filename);
        textures[i + j].id = this->texture_from_file(filename.C_Str(), this->directory);
        textures[i + j].type = SPECULAR;
        //texture.filename = filename; // What's this?
    }

    return textures;
}


GLuint Model::texture_from_file(const char *filename, const char *directory)
{
    char filepath[PATH_MAX];
    strncpy(filepath, directory, sizeof(filepath) - 1);
    strcat(filepath, "/");
    strncat(filepath, filename, sizeof(filepath) - strlen(filepath));

    GLuint texture_id;
    glGenTextures(1, &texture_id);

    int width, height;
    GLubyte *data = SOIL_load_image(filepath, &width, &height, NULL, SOIL_LOAD_RGB);

    if (data == NULL) {
        throw "Failed to load texture";
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    SOIL_free_image_data(data);
    return texture_id;
}


void Model::draw(const ShaderProgram& shader_program,
                 const Camera& camera, const glm::mat4& model) const
{
    for (unsigned int i = 0; i < num_meshes; i++) {
        meshes[i].draw(shader_program, camera, model);
    }
}

void Model::outline(const ShaderProgram& shader_program,
                    const Camera& camera, glm::mat4 model, GLfloat thickness) const
{
    for (unsigned int i = 0; i < num_meshes; i++) {
        meshes[i].outline(shader_program, camera, model, thickness);
    }
}
