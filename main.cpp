#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <SOIL/SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#include "shader_program.hpp"
#include "camera.hpp"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600

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

static void key_callback(
        GLFWwindow *window,
        int key, int scancode, int action, int mods);

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

        ShaderProgram shader_program(
                "shaders/shader.vert", "shaders/shader.frag");
        ShaderProgram lighting(
                "shaders/lighting.vert", "shaders/lighting.frag");
        lighting.use();
        lighting.set_uniform3fv(
                "object_color", glm::value_ptr(glm::vec3(0.4f, 0.2f, 0.3f)));
        lighting.set_uniform3fv(
                "light_color", glm::value_ptr(glm::vec3(1.0f)));

        ShaderProgram light_source(
                "shaders/light_source.vert", "shaders/light_source.frag");

        GLfloat vertices[] = {
            // position                      // texture coords
            -0.50000f, -0.28868f, -0.27216f, 0.0f, 0.0f,
             0.00000f,  0.57735f, -0.27216f, 0.5f, 1.0f,
             0.50000f, -0.28868f, -0.27216f, 1.0f, 0.0f,

            -0.50000f, -0.28868f, -0.27216f, 0.0f, 0.0f,
             0.00000f,  0.57735f, -0.27216f, 0.5f, 1.0f,
             0.00000f,  0.00000f,  0.54433f, 1.0f, 0.0f,

             0.00000f,  0.57735f, -0.27216f, 0.0f, 0.0f,
             0.50000f, -0.28868f, -0.27216f, 0.5f, 1.0f,
             0.00000f,  0.00000f,  0.54433f, 1.0f, 0.0f,

            -0.50000f, -0.28868f, -0.27216f, 0.0f, 0.0f,
             0.50000f, -0.28868f, -0.27216f, 0.5f, 1.0f,
             0.00000f,  0.00000f,  0.54433f, 1.0f, 0.0f
        };

        GLfloat cube[] = {
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,

            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,

            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
        };

        glm::vec3 positions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };

        GLint width, height;
        GLubyte *image = SOIL_load_image(
                "textures/brown_wood.png", &width, &height, 0, SOIL_LOAD_RGB);
        if (image == NULL) {
            fprintf(stderr, "Image not loaded\n");
            perror("SOIL");
            exit(1);
        }
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *) image);
        glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        GLuint vao[2];
        glGenVertexArrays(2, vao);
        GLuint vbo[2];
        glGenBuffers(2, vbo);

        /* pyramid */
        glBindVertexArray(vao[0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(
                GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(
                0, 3, GL_FLOAT, GL_FALSE,
                5 * sizeof(GLfloat), (GLvoid *) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(
                1, 2, GL_FLOAT, GL_FALSE,
                5 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        /* cube */
        glBindVertexArray(vao[1]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(
                GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

        glVertexAttribPointer(
                0, 3, GL_FLOAT, GL_FALSE,
                3 * sizeof(GLfloat), (GLvoid *) 0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            glm::mat4 view = camera.get_view_matrix();
            glm::mat4 projection = camera.get_projection_matrix();

            shader_program.use();
            shader_program.set_uniform_matrix4fv(
                    "projection", glm::value_ptr(projection));
            shader_program.set_uniform_matrix4fv(
                    "view", glm::value_ptr(view));
            glm::mat4 model = glm::mat4(1.0f);
            for(unsigned i = 0; i < sizeof(positions) / sizeof(positions[0]); i++) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, positions[i]);
                model = glm::scale(model, glm::vec3(2.0f));
                model = glm::rotate(
                        model,
                        glm::radians(12.0f * i),
                        glm::vec3(1.0f, 0.3f, 0.5f));

                lighting.use();
                lighting.set_uniform_matrix4fv(
                        "projection", glm::value_ptr(projection));
                lighting.set_uniform_matrix4fv(
                        "view", glm::value_ptr(view));
                lighting.set_uniform_matrix4fv(
                        "model", glm::value_ptr(model));

                glBindTexture(GL_TEXTURE_2D, texture);
                glBindVertexArray(vao[0]);
                glDrawArrays(GL_TRIANGLES, 0, 12);
            }
            light_source.use();
            light_source.set_uniform_matrix4fv(
                    "projection", glm::value_ptr(projection));
            light_source.set_uniform_matrix4fv(
                    "view", glm::value_ptr(view));
            model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.2f));
            light_source.set_uniform_matrix4fv(
                    "model", glm::value_ptr(model));
            glBindVertexArray(vao[1]);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);

            glfwSwapBuffers(window);
            GLfloat current_time = glfwGetTime();
            deltatime = current_time - last_frame_time;
            last_frame_time = current_time;
        }

        glDeleteVertexArrays(2, vao);
        glDeleteBuffers(2, vbo);
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
