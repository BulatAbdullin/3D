#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

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

static Camera camera((GLfloat) WINDOW_HEIGHT, (GLfloat) WINDOW_HEIGHT);
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

static void key_callback(GLFWwindow *window,
                         int key,
                         int scancode,
                         int action,
                         int mods);

static void mouse_callback(GLFWwindow *window, double xpos, double ypos);

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

        Model object("models/cube.obj");
        ShaderProgram shader_program(
                "shaders/phong.vert", "shaders/phong.frag");

#if 1
        // Texture
        GLuint diffuse_map;
        glGenTextures(1, &diffuse_map);

        glActiveTexture(0);
        glBindTexture(GL_TEXTURE_2D, diffuse_map);
        // load and generate the texture
        int width, height;
        GLubyte *data = SOIL_load_image("textures/brown_wood.png", &width, &height, NULL, SOIL_LOAD_RGB);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            throw "Failed to load texture";
        }
        SOIL_free_image_data(data);
        glBindTexture(GL_TEXTURE_2D, 0);
#endif

        glEnable(GL_DEPTH_TEST); /* enable depth testing so that objects do not overlap */
        glEnable(GL_STENCIL_TEST);
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            glClearColor(0.1f, 0.0f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            glm::mat4 view = camera.get_view_matrix();
            glm::mat4 projection = camera.get_projection_matrix();

            GLfloat time = glfwGetTime();
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            model = glm::rotate(
                    model,
                    glm::radians(25.0f * time),
                    glm::vec3(0.4f, 1.0f, 0.3f));

            shader_program.use();
            shader_program.set_uniform_matrix4fv(
                    "projection", glm::value_ptr(projection));
            shader_program.set_uniform_matrix4fv(
                    "view", glm::value_ptr(view));
            shader_program.set_uniform_matrix4fv(
                    "model", glm::value_ptr(model));

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
                    "light.diffuse", glm::value_ptr(glm::vec3(0.5f)));
            shader_program.set_uniform3fv(
                    "light.specular", glm::value_ptr(glm::vec3(1.0f)));

#if 1
            // Material
            shader_program.set_uniform3fv(
                    "material.specular",
                    glm::value_ptr(glm::vec3(0.5f)));
            shader_program.set_uniform1f("material.shininess", 32.0f);

            glBindTexture(GL_TEXTURE_2D, diffuse_map);
#endif

            object.draw(shader_program);

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
