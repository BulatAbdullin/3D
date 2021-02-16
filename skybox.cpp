#include "skybox.hpp"

/* Some constants */

const GLint Skybox::num_faces = 6;

const GLfloat Skybox::vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

const GLint Skybox::num_vertices = sizeof(Skybox::vertices) / sizeof(GLfloat);


Skybox::Skybox(const char **image_files) /* filepaths: right left top bottom back front */
        : shader("shaders/skybox.vert", "shaders/skybox.frag")
{
    this->setup_cubemap(image_files);
    this->setup_VAO();
    shader.set_uniform1i("cubemap", 0);
}


void Skybox::setup_cubemap(const char **image_files)
{
    glGenTextures(1, &cubemap_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);

    for (int i = 0; i < num_faces; i++) {
        int width, height;
        GLubyte *img = (GLubyte *) SOIL_load_image(image_files[i],
                                                   &width, &height, 0, SOIL_LOAD_RGB);
        if (img == 0) {
            throw "Failed to load cubemap";
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                     width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        /*
         *   GL_TEXTURE_CUBE_MAP_POSITIVE_X    Right
         *   GL_TEXTURE_CUBE_MAP_NEGATIVE_X    Left
         *   GL_TEXTURE_CUBE_MAP_POSITIVE_Y    Top
         *   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    Bottom
         *   GL_TEXTURE_CUBE_MAP_POSITIVE_Z    Back
         *   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    Front
         */
        SOIL_free_image_data(img);
    }

    /* Some magic */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


void Skybox::setup_VAO()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Skybox::vertices), Skybox::vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


Skybox::~Skybox()
{
    glDeleteTextures(1, &cubemap_id);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}


void Skybox::draw(const Camera& camera) const
{
    this->shader.use();
    this->shader.update(camera, glm::mat4(1.0f));

    this->bind();
    glDepthFunc(GL_LEQUAL);

    glDrawArrays(GL_TRIANGLES, 0 /* first */, Skybox::num_vertices);

    this->unbind();
    glDepthFunc(GL_LESS);
}


void Skybox::bind() const
{
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
}


void Skybox::unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
}
