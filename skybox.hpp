#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <GL/gl.h>
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>

#include "shader_program.hpp"
#include "camera.hpp"


class Skybox
{
private:
    GLuint VAO;
    GLuint VBO;

    GLuint cubemap_id;
    ShaderProgram shader;

    const static GLint num_faces;
    const static GLint num_vertices;
    const static GLfloat vertices[];

    void setup_cubemap(const char **image_files);

    void setup_VAO();

    void bind() const;

    void unbind() const;

public:
    Skybox(const char **image_files); /* filepaths: right left top bottom back front */

    ~Skybox();

    void draw(const Camera& camera) const;

    inline GLuint get_cubemap_id()
    {
        return this->cubemap_id;
    }
};


#endif /* ifndef SKYBOX_HPP */
