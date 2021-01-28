#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

class ShaderProgram
{
private:
    GLuint id;
    const static GLuint info_log_size = 512;
public:
    ShaderProgram(const char *vertex_shader_source_filepath,
                  const char *fragment_shader_source_filepath);

    void use() const;

    void set_uniform1f(const char *name, GLfloat value) const 
    {
        glUniform1f(glGetUniformLocation(id, name), value);
    }

    void set_uniform1i(const char *name, GLint value) const 
    {
        glUniform1i(glGetUniformLocation(id, name), value);
    }

    void set_uniform3fv(const char *name, const GLfloat *value) const
    {
        glUniform3fv(glGetUniformLocation(id, name), 1, value);
    }

    void set_uniform_matrix4fv(const char *name, const GLfloat *value) const
    {
        glUniformMatrix4fv(
                glGetUniformLocation(id, name),
                1 /* how many matrices */,
                GL_FALSE /* do not transpose */,
                value
        );
    }

    GLuint get_id() const
    {
        return id;
    }
};

#endif /* ifndef SHADER_PROGRAM_HPP */
