#ifndef SHADER_HPP
#define SHADER_HPP

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

template <GLenum SHADER_TYPE>
class Shader
{
private:
    GLuint id;
    const static GLuint info_log_size = 512;
public:
    Shader(const char *source_filepath);

    static const char *read_source(const char *source_filepath);

    GLuint get_id()
    {
        return id;
    }

    ~Shader();
};


/* Implementation */

template <GLenum SHADER_TYPE>
Shader<SHADER_TYPE>::Shader(const char *source_filepath)
    : id(glCreateShader(SHADER_TYPE))
{
    const char *shader_source = read_source(source_filepath);
    glShaderSource(id, 1, &shader_source, NULL);
    delete[] shader_source;
    glCompileShader(id);
    GLint compilation_success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compilation_success);
    if (!compilation_success) {
        GLchar *info_log = new GLchar[info_log_size];
        glGetShaderInfoLog(id, info_log_size, NULL, info_log);
        throw info_log;
    }
}

template <GLenum SHADER_TYPE>
const char *Shader<SHADER_TYPE>::read_source(const char *source_filepath)
{
    FILE *f = fopen(source_filepath, "r");
    if (!f) {
        throw strerror(errno);
    }
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = new char[filesize + 1];
    long offset = 0;
    while (offset < filesize) {
        fread(buf + offset, sizeof(char), filesize, f);
        offset = ftell(f);
    }
    buf[filesize] = '\0';
    fclose(f);
    return buf;
}

template <GLenum SHADER_TYPE>
Shader<SHADER_TYPE>::~Shader()
{
    glDeleteShader(id);
}

#endif /* ifndef SHADER_HPP */
