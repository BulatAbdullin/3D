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
#include "skybox.hpp"

#include "square.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 700

static Camera camera((GLfloat) WINDOW_HEIGHT, (GLfloat) WINDOW_HEIGHT, glm::vec3(0.0f));
static GLfloat deltatime = 0.0f; // time in which last frame was rendered
static GLfloat last_frame_time = 0.0f;

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

static GLuint load_texture_grayscale(const char *img_path);


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
        GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "GLFW", NULL, NULL);

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
        ShaderProgram light_source_shader("shaders/light_source.vert",
                                          "shaders/light_source.frag");
        ShaderProgram mirror_faces_shader("shaders/reflect.vert", "shaders/reflect.frag");
        ShaderProgram refraction_shader("shaders/refract.vert", "shaders/refract.frag");

        GLuint square_VAO = load_square();
        Model square("models/square.obj");

        /* Skybox */
        const char *skybox_image_files[] = {
            "textures/skybox/posx.png",
            "textures/skybox/negx.png",
            "textures/skybox/posy.png",
            "textures/skybox/negy.png",
            "textures/skybox/posz.png",
            "textures/skybox/negz.png"
        };

        Skybox skybox(skybox_image_files);

        /* Normal mapping + Parallax mapping */
        ShaderProgram relief_mapping_shader("shaders/relief_mapping.vert",
                                            "shaders/relief_mapping.frag");
        relief_mapping_shader.set_uniform1i("diffuse_map", 0);
        relief_mapping_shader.set_uniform1i("normal_map", 1);
        relief_mapping_shader.set_uniform1i("displacement_map", 2);
        relief_mapping_shader.set_uniform1i("specular_map", 3);

#if 1
        GLuint diffuse_map = load_texture_rgb("textures/bricks.png");
        GLuint normal_map = load_texture_rgb("textures/bricks_normal.png");
        GLuint displacement_map = load_texture_grayscale("textures/bricks_disp.png");
        GLuint specular_map = load_texture_rgb("textures/cement.png");
#else
        GLuint diffuse_map = load_texture_rgb("textures/white.png");
        GLuint normal_map = load_texture_rgb("textures/toy_box_normal.png");
        GLuint displacement_map = load_texture_grayscale("textures/toy_box_disp.png");
        GLuint specular_map = load_texture_rgb("textures/cement.png");
#endif

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            glClearColor(0.1f, 0.0f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST); /* enable depth testing so that objects do not overlap */


            GLfloat time = glfwGetTime();
            float radius = 2.0f;
            glm::vec3 light_position(radius * glm::cos(time), 2.0f, radius * glm::sin(time));

            glm::mat4 model;
            model = glm::mat4(1.0f);
            model = glm::translate(model, light_position);
            model = glm::scale(model, glm::vec3(0.3f));

            cube.draw(light_source_shader, camera, model);

            relief_mapping_shader.use();
            /* needed for specular component in Phong shading */
            relief_mapping_shader.set_uniform3fv(
                    "view_position", glm::value_ptr(camera.get_position()));

            // Light
            relief_mapping_shader.set_uniform3fv("light_position", glm::value_ptr(light_position));
            relief_mapping_shader.set_uniform3fv("light.ambient",
                                                 glm::value_ptr(glm::vec3(0.15f)));
            relief_mapping_shader.set_uniform3fv("light.diffuse",
                                                 glm::value_ptr(glm::vec3(0.8f)));
            relief_mapping_shader.set_uniform3fv("light.specular",
                                                 glm::value_ptr(glm::vec3(0.2)));

            // Material
            relief_mapping_shader.set_uniform1f("material.shininess", 16.0f);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));


            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_map);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normal_map);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, displacement_map);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, specular_map);
            draw_square(square_VAO, relief_mapping_shader, camera, model);

            /* Semitransparent cubes */
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.get_cubemap_id());

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-5.0f, 2.0f, -4.0f));
            model = glm::rotate(model, time, glm::vec3(0.1f, 0.4f, 0.2f));

            /* refraction */
            cube.draw(refraction_shader, camera, model);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(5.0f, 2.0f, -4.0f));
            model = glm::rotate(model, time, glm::vec3(0.1f, 0.4f, 0.2f));
            /* refrection */
            cube.draw(mirror_faces_shader, camera, model);

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

            /* Skybox */
            skybox.draw(camera);

            glfwSwapBuffers(window);

            GLfloat current_time = glfwGetTime();
            deltatime = current_time - last_frame_time;
            last_frame_time = current_time;
        }

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

    if (img == 0) {
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

static GLuint load_texture_grayscale(const char *img_path)
{
    int width, height;
    GLubyte *img = SOIL_load_image(img_path, &width, &height, NULL /* channels */, SOIL_LOAD_L);

    if (img == NULL) {
        throw "Failed to load texture";
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D /* texture target */,
                 0 /* mipmap level */,
                 GL_RED /* in what format to store the image */,
                 width, height,
                 0 /* for some reason should always be zero */,
                 GL_RED /* format of a source image */,
                 GL_UNSIGNED_BYTE /* datatype of a source image */,
                 img /* image data */);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    SOIL_free_image_data(img);

    return id;
}
