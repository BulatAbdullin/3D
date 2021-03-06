#include <string.h> /* memcpy */
#include <stdio.h>  /* snprintf */
#include <stddef.h> /* offsetof(type, member) */

#include "mesh.hpp"

Mesh::Mesh(const Vertex *vertices, int num_vertices,
           const GLuint *indices, int num_indices,
           const Texture *textures, int num_textures)
    : num_vertices(num_vertices)
    , num_indices(num_indices)
    , textures(new Texture[num_textures])
    , num_textures(num_textures)
{
    memcpy(this->textures, textures, num_textures);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // copy vertices array to a vertex buffer
    glBufferData(GL_ARRAY_BUFFER,
                 num_vertices * sizeof(Vertex),
                 vertices,
                 GL_STATIC_DRAW);
    setup_attrib_pointers();

    // copy indices array to an element buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 num_indices * sizeof(GLuint),
                 indices,
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
}


void Mesh::setup_attrib_pointers() const
{
    // position
    glVertexAttribPointer(0, /* (location = 0) */
                          3, /* vec3 */
                          GL_FLOAT, /* data type */
                          GL_FALSE, /* do not normalize the data */
                          sizeof(Vertex), /* stride */
                          (void *) 0 /* offset */);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, /* (location = 1) */
                          3, /* vec3 */
                          GL_FLOAT, /* data type */
                          GL_FALSE, /* do not normalize the data */
                          sizeof(Vertex), /* stride */
                          (void *) offsetof(Vertex, normal) /* offset */);
    glEnableVertexAttribArray(1);

    // texture coordinates
    glVertexAttribPointer(2, /* (location = 2) */
                          2, /* vec2 */
                          GL_FLOAT, /* data type */
                          GL_FALSE, /* do not normalize the data */
                          sizeof(Vertex), /* stride */
                          (void *) offsetof(Vertex, texture_coords) /* offset */);
    glEnableVertexAttribArray(2);

}


void Mesh::draw(const ShaderProgram& shader_program,
                const Camera& camera, const glm::mat4& model) const
{
    shader_program.update(camera, model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void Mesh::outline(const ShaderProgram& shader_program,
                   const Camera& camera, glm::mat4 model, GLfloat thickness) const
{
    // Stencil testing
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, /* stencil test fails */
                GL_KEEP, /* stencil test passes, depth test fails */
                GL_REPLACE /* stencil and depth tests pass */);

    /* all the object's fragments should pass the stencil test */
    glStencilFunc(GL_ALWAYS, 1 /* reference value for the stencil test */, 0xFF /* mask */);
    glStencilMask(0xFF); /* enable writing to the stencil buffer */
    shader_program.use();
    this->draw(shader_program, camera, model);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00); /* disable writing to the stencil buffer */
    glDisable(GL_DEPTH_TEST);
    ShaderProgram outlining_shader("shaders/outline.vert", "shaders/outline.frag");
    outlining_shader.use();

    // draw scaled up
    model = glm::scale(model, glm::vec3(thickness));
    this->draw(outlining_shader, camera, model);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glEnable(GL_DEPTH_TEST);
}


void Mesh::set_material(const ShaderProgram& shader_program) const
{
    shader_program.use();
    int i_diffuse = 1, i_specular = 1;
    for(int i = 0; i < num_textures; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        char uniform_sampler2D_name[32];
        if (textures[i].type == DIFFUSE) {
            snprintf(uniform_sampler2D_name, sizeof(uniform_sampler2D_name),
                     "material.texture_diffuse%d", i_diffuse);
            i_diffuse++;
        } else if (textures[i].type == SPECULAR) {
            snprintf(uniform_sampler2D_name, sizeof(uniform_sampler2D_name),
                     "material.texture_specular%d", i_specular);
            i_specular++;
        }
        shader_program.set_uniform1i(uniform_sampler2D_name, i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);
}
