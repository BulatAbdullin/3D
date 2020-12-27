#include "shader_program.hpp"
#include "shader.hpp"

ShaderProgram::ShaderProgram(
        const char *vertex_shader_source_filepath,
        const char *fragment_shader_source_filepath)
    : id(glCreateProgram())
{
    Shader<GL_VERTEX_SHADER> vertex_shader(vertex_shader_source_filepath);
    Shader<GL_FRAGMENT_SHADER> fragment_shader(fragment_shader_source_filepath);
    glAttachShader(id, vertex_shader.get_id());
    glAttachShader(id, fragment_shader.get_id());
    glLinkProgram(id);
    GLint link_success;
    glGetProgramiv(id, GL_LINK_STATUS, &link_success);
    if (!link_success) {
        GLchar *info_log = new GLchar[info_log_size];
        glGetProgramInfoLog(id, info_log_size, NULL, info_log);
        throw info_log;
    }
}

void ShaderProgram::use() const
{
    glUseProgram(id);
}
