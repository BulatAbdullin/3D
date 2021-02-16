#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <GL/gl.h>

#include <SOIL/SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <map>

#include "shader_program.hpp"
#include "camera.hpp"
#include "model.hpp"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 700

static Camera camera((GLfloat) WINDOW_HEIGHT, (GLfloat) WINDOW_HEIGHT,
                     glm::vec3(-10.0f, 3.0f, 0.0f));
static GLfloat deltatime = 0.0f; // time in which last frame was rendered
static GLfloat last_frame_time = 0.0f;

static bool grayscale = false;

// mouse
static bool is_first_mouse_move = true;
static struct cursor_position
{
    GLfloat x = (GLfloat) WINDOW_WIDTH / 2.0f;
    GLfloat y = (GLfloat) WINDOW_HEIGHT / 2.0f;
} cursor_position;

static void error_callback(int error, const char *description);

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

static GLuint load_texture_rgb(const char *img_path);
static GLuint load_texture_rgba(const char *img_path);

static void draw_scene(const ShaderProgram&);

int main(int argc, char *argv[])
{
    try {
        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
            exit(1);
        }
        // OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_DECORATED, GL_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        GLFWwindow *window = glfwCreateWindow(
                WINDOW_WIDTH, WINDOW_HEIGHT, "GLFW", NULL, NULL);
        if (!window) {
            glfwTerminate();
            exit(1);
        }
        glfwMakeContextCurrent(window);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glfwSetKeyCallback(window, key_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);

        Model cube("models/cube.obj");
        ShaderProgram shader_program(
                "shaders/phong.vert", "shaders/phong.frag");
        Model floor("models/square.obj");
        GLuint floor_texture = load_texture_rgb("textures/stones.png");

        Model billboard("models/square.obj");
        ShaderProgram blend("shaders/blend.vert", "shaders/blend.frag");

        // Texture
        GLuint diffuse_map = load_texture_rgb("textures/tiles.png");

        // Billboard
        GLuint billboard_texture = load_texture_rgba("textures/broken_window.png");

        glm::vec3 billboard_positions[] = {
            glm::vec3(-1.5f, 0.0f, -0.48f),
            glm::vec3( 1.5f, 0.0f, 0.51f),
            glm::vec3( 0.0f, 0.0f, 0.7f),
            glm::vec3(-0.3f, 0.0f, -2.3f),
            glm::vec3( 0.5f, 0.0f, -0.6f)
        };

        glStencilFunc(GL_EQUAL, 1, 0xFF);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // screen quad VAO
        GLfloat screen_vertices[] = {
            // positions   // texture_coords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        ShaderProgram screen_shader("shaders/screen.vert", "shaders/screen.frag");

        GLuint screenVAO, screenVBO;
        glGenVertexArrays(1, &screenVAO);
        glGenBuffers(1, &screenVBO);
        glBindVertexArray(screenVAO);
        glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertices), &screen_vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) (2 * sizeof(GLfloat)));

        /* 2D Post-processing */
        GLuint FBO; // Framebuffer object
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        // create a color attachment texture
        GLuint colorbuffer_texture;
        glGenTextures(1, &colorbuffer_texture);
        glBindTexture(GL_TEXTURE_2D, colorbuffer_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
#if 1
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               colorbuffer_texture, 0);

        GLuint RBO; // Renderbuffer Object
        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw "Framebuffer is incomplete";
        }
        /* END 2D Post-processing */


#if 0
        /* Shadow map configuration */
        GLuint shadow_map_fbo; // framebufer
        glGenFramebuffers(1, &shadow_map_fbo);

        const GLuint shadow_map_width = 2048, shadow_map_height = 2048;
        GLuint shadow_map_texture;
        glGenTextures(1, &shadow_map_texture);
        glBindTexture(GL_TEXTURE_2D, shadow_map_texture);
        glTexImage2D(GL_TEXTURE_2D, 0 /* mipmap level */, GL_DEPTH_COMPONENT,
                     shadow_map_width, shadow_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        /* some magic here */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                               shadow_map_texture, 0);
        /* We will not need a color buffer */
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        /* END Shadow map configuration */
#endif


        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // bind to framebuffer and draw scene as we normally would to color texture
            if (grayscale) {
                glBindFramebuffer(GL_FRAMEBUFFER, FBO);
            }
            glClearColor(0.1f, 0.0f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST); /* enable depth testing so that objects do not overlap */

            GLfloat time = glfwGetTime();

            /* needed for specular component in Phong shading */
            shader_program.set_uniform3fv(
                    "view_position", glm::value_ptr(camera.get_position()));

            // Light
            shader_program.use();
            shader_program.set_uniform3fv(
                    "light.position",
                    glm::value_ptr(glm::vec3(5.0f, 1.0f, 5.0f)));
            shader_program.set_uniform3fv(
                    "light.ambient", glm::value_ptr(glm::vec3(0.1f)));
            shader_program.set_uniform3fv(
                    "light.diffuse", glm::value_ptr(glm::vec3(0.4f)));
            shader_program.set_uniform3fv(
                    "light.specular", glm::value_ptr(glm::vec3(0.5f)));

            // Material
            shader_program.set_uniform3fv(
                    "material.specular",
                    glm::value_ptr(glm::vec3(0.5f)));
            shader_program.set_uniform1f("material.shininess", 32.0f);

            glm::mat4 model;

            model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(50.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::translate(model, glm::vec3(-0.3f, 0.0f, 0.1f));
            glBindTexture(GL_TEXTURE_2D, floor_texture);
            glStencilMask(0x00);
            floor.draw(shader_program, camera, model);
            //glStencilMask(0xFF);


            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(9.0f, -3.0f, -1.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            model = glm::rotate(model, glm::radians(50.0f + 25.0f * time),
                                glm::vec3(0.4f, 1.0f, 0.3f));
            glBindTexture(GL_TEXTURE_2D, diffuse_map);
            cube.draw(shader_program, camera, model);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(9.0f, 2.0f, 1.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            model = glm::rotate(model, glm::radians(25.0f * time), glm::vec3(0.4f, 1.0f, 0.3f));

            glBindTexture(GL_TEXTURE_2D, diffuse_map);
            cube.outline(shader_program, camera, model, 1.0f + 0.03f);


            /* too slow */
#if 0
            qsort(billboard_positions,
                  sizeof(billboard_positions) / sizeof(billboard_positions[0]),
                  sizeof(billboard_positions[0]),
                  billboard_dist_compare);

            glBindTexture(GL_TEXTURE_2D, billboard_texture);
            for (size_t i = 0; i < sizeof(billboard_positions) / sizeof(billboard_positions[0]);
                    i++) {
                model = glm::mat4(1.0f);
                model = glm::scale(model, glm::vec3(2.0f));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, billboard_positions[i]);
                billboard.draw(blend, camera, model);
            }
#else
            std::map<GLfloat, glm::vec3> sorted_positions;
            for (unsigned int i = 0;
                    i < sizeof(billboard_positions) / sizeof(billboard_positions[0]); i++) {
                GLfloat distance = glm::length(camera.get_position() - billboard_positions[i]);
                sorted_positions[distance] = billboard_positions[i];
            }
            glBindTexture(GL_TEXTURE_2D, billboard_texture);
            for (std::map<GLfloat, glm::vec3>::reverse_iterator it = sorted_positions.rbegin();
                    it != sorted_positions.rend(); it++) {
                model = glm::mat4(1.0f);
                model = glm::scale(model, glm::vec3(2.0f));
                model = glm::translate(model, it->second);
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                billboard.draw(blend, camera, model);
            }
#endif

            if (grayscale) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDisable(GL_DEPTH_TEST);

                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                screen_shader.use();
                glBindVertexArray(screenVAO);
                glBindTexture(GL_TEXTURE_2D, colorbuffer_texture);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            glfwSwapBuffers(window);

            GLfloat current_time = glfwGetTime();
            deltatime = current_time - last_frame_time;
            last_frame_time = current_time;
        }

        glDeleteVertexArrays(1, &screenVAO);
        glDeleteBuffers(1, &screenVBO);

        glfwDestroyWindow(window);
        glfwTerminate();
        
    } catch (const char *err_message) {
        fprintf(stderr, err_message);
    }
    return 0;
}


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(
        GLFWwindow *window,
        int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    /* toggle black and white post-processing effect */
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        grayscale = !grayscale;
        return;
    }

    /* camera movement */
    switch (key)
    {
        case GLFW_KEY_W:
            camera.move_forward(deltatime);
            break;
        case GLFW_KEY_S:
            camera.move_backward(deltatime);
            break;
        case GLFW_KEY_A:
            camera.move_left(deltatime);
            break;
        case GLFW_KEY_D:
            camera.move_right(deltatime);
            break;
        case GLFW_KEY_U:
            camera.move_up(deltatime);
            break;
        case GLFW_KEY_P:
            camera.move_down(deltatime);
            break;
        default:
            break;
    }
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (is_first_mouse_move) {
        cursor_position.x = xpos;
        cursor_position.y = ypos;
        is_first_mouse_move = false;
    }

    GLfloat xoffset = xpos - cursor_position.x;
    GLfloat yoffset = ypos - cursor_position.y;
    cursor_position.x = xpos;
    cursor_position.y = ypos;
    camera.process_mouse_movement(xoffset, yoffset);
}

static GLuint load_texture_rgb(const char *img_path)
{
    int width, height;
    GLubyte *img = SOIL_load_image(img_path, &width, &height, NULL /* channels */, SOIL_LOAD_RGB);

    if (img == NULL) {
        throw "Failed to load texture";
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D /* texture target */,
                 0 /* mipmap level */,
                 GL_RGB /* in what format to store the image */,
                 width, height,
                 0 /* for some reason should always be zero */,
                 GL_RGB /* format of a source image */,
                 GL_UNSIGNED_BYTE /* datatype of a source image */,
                 img /* image data */);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    SOIL_free_image_data(img);

    return id;
}

static GLuint load_texture_rgba(const char *img_path)
{
    int width, height;
    GLubyte *img = SOIL_load_image(img_path, &width, &height, NULL /* channels */, SOIL_LOAD_RGBA);

    if (img == NULL) {
        throw "Failed to load texture";
    }

    GLuint id;
    glGenTextures(1, &id);
    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D /* texture target */,
                 0 /* mipmap level */,
                 GL_RGBA /* in what format to store the image */,
                 width, height,
                 0 /* for some reason should always be zero */,
                 GL_RGBA /* format of a source image */,
                 GL_UNSIGNED_BYTE /* datatype of a source image */,
                 img /* image data */);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    SOIL_free_image_data(img);

    return id;
}
