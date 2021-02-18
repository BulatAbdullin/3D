#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <GL/gl.h>

#include <SOIL/SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#include "shader_program.hpp"
#include "camera.hpp"
#include "model.hpp"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 700

static Camera camera((GLfloat) WINDOW_HEIGHT, (GLfloat) WINDOW_HEIGHT,
                     glm::vec3(0.0f, 2.0f, 5.0f));

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

// static void draw_scene(const ShaderProgram&);
static int billboard_dist_compare(const void *, const void *);


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
        ShaderProgram phong_shader("shaders/phong.vert", "shaders/phong.frag");
        ShaderProgram light_source_shader("shaders/light_source.vert",
                                          "shaders/light_source.frag");
        GLuint diffuse_map = load_texture_rgb("textures/tiles.png");

        Model floor("models/square.obj");
        GLuint floor_texture = load_texture_rgb("textures/stones.png");

        Model billboard("models/square.obj");
        ShaderProgram blend("shaders/blend.vert", "shaders/blend.frag");


        // Billboard
        GLuint billboard_texture = load_texture_rgba("textures/broken_window.png");

        glm::vec3 billboard_positions[] = {
            glm::vec3(-0.7f, 0.5f, 0.0f),
            glm::vec3( 0.7f, 0.5f, 0.0f),
            glm::vec3( 0.0f, 0.5f, 1.0f),
            glm::vec3( 0.0f, 0.5f, -6.0f)
        };

        glStencilFunc(GL_EQUAL, 1, 0xFF);

        /* blending */
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /* 2D post-processing effect */
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertices), screen_vertices, GL_STATIC_DRAW);
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
        glTexImage2D(GL_TEXTURE_2D, 0 /* mipmap level */, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, 0 /* empty */);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER /* target */,
                              GL_COLOR_ATTACHMENT0 /* attachment */,
                              GL_TEXTURE_2D /* texture target */,
                              colorbuffer_texture /* texture */, 0 /* mipmap level */);

        // depth and stecil attachments
        GLuint depth24_stencil8_buffer_texture;
        glGenTextures(1, &depth24_stencil8_buffer_texture);
        glBindTexture(GL_TEXTURE_2D, depth24_stencil8_buffer_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D, depth24_stencil8_buffer_texture, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* End 2D Post-processing */


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

            // Light
            glm::vec3 light_position(5.0f, 3.0f, 9.0f);
            light_position = glm::vec3(8.0f * glm::cos(time), 8.0f, 8.0 * glm::sin(time));
            phong_shader.use();
            phong_shader.set_uniform3fv(
                    "light.position", glm::value_ptr(light_position));
            phong_shader.set_uniform3fv(
                    "light.ambient", glm::value_ptr(glm::vec3(0.1f)));
            phong_shader.set_uniform3fv(
                    "light.diffuse", glm::value_ptr(glm::vec3(0.4f)));
            phong_shader.set_uniform3fv(
                    "light.specular", glm::value_ptr(glm::vec3(0.5f)));

            // Material
            phong_shader.set_uniform3fv(
                    "material.specular", glm::value_ptr(glm::vec3(0.5f)));
            phong_shader.set_uniform1f("material.shininess", 32.0f);

            glm::mat4 model;


            model = glm::mat4(1.0f);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(50.0f));
            glBindTexture(GL_TEXTURE_2D, floor_texture);
            glStencilMask(0x00);
            floor.draw(phong_shader, camera, model);
            //glStencilMask(0xFF);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 3.0f, -2.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            model = glm::rotate(model, glm::radians(50.0f + 25.0f * time),
                                glm::vec3(0.4f, 1.0f, 0.3f));
            glBindTexture(GL_TEXTURE_2D, diffuse_map);
            cube.draw(phong_shader, camera, model);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(9.0f, 2.0f, 1.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            model = glm::rotate(model, glm::radians(-25.0f * time), glm::vec3(0.4f, 1.0f, 0.3f));

            glBindTexture(GL_TEXTURE_2D, diffuse_map);
            cube.outline(phong_shader, camera, model, 1.0f + 0.03f);

            /* display light source */
            model = glm::mat4(1.0f);
            model = glm::translate(model, light_position);
            model = glm::scale(model, glm::vec3(0.5f));
            cube.draw(light_source_shader, camera, model);


            /* sort billboards from nearest to furthest */
            qsort(billboard_positions,
                  sizeof(billboard_positions) / sizeof(billboard_positions[0]),
                  sizeof(billboard_positions[0]),
                  billboard_dist_compare);

            glBindTexture(GL_TEXTURE_2D, billboard_texture);

            /* back to front */
            int num_billboards = sizeof(billboard_positions) / sizeof(billboard_positions[0]);
            for (int i = num_billboards - 1; i >= 0; i--) {
                model = glm::mat4(1.0f);
                model = glm::scale(model, glm::vec3(2.0f));
                model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
                model = glm::translate(model, billboard_positions[i]);
                billboard.draw(blend, camera, model);
            }

            if (grayscale) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                /* it's  a flat screen, there's no need for depth testing because it must always
                 * be visible */
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

static void mouse_callback(GLFWwindow *window, double x, double y)
{
    if (is_first_mouse_move) {
        cursor_position.x = x;
        cursor_position.y = y;
        is_first_mouse_move = false;
    }

    GLfloat xoffset = x - cursor_position.x;
    GLfloat yoffset = y - cursor_position.y;
    cursor_position.x = x;
    cursor_position.y = y;
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

static int billboard_dist_compare(const void *pos1, const void *pos2)
{
    GLfloat dist_diff = glm::length(camera.get_position() - *(glm::vec3 *) pos1)
                      - glm::length(camera.get_position() - *(glm::vec3 *) pos2);

    if (dist_diff < 0.0f) {
        return -1;
    } else if (dist_diff > 0.0f) {
        return 1;
    }

    return 0;
}
