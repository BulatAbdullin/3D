#ifndef CAMERA_HPP
#define CAMERA_HPP

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
    GLfloat width;
    GLfloat height;
    GLfloat near;
    GLfloat far;
    GLfloat fov; /* field of view */

    GLfloat speed;
    GLfloat mouse_sensitivity;

    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 right;

    GLfloat yaw;
    GLfloat pitch;

    const static GLfloat pitch_max;
    const static GLfloat pitch_min;
    const static GLfloat fov_max;
    const static GLfloat fov_min;

    void update();
public:
    Camera(
            GLfloat width,
            GLfloat height,
            GLfloat near              = 0.1f,
            GLfloat far               = 100.0f,
            GLfloat fov               = 45.0f,
            GLfloat speed             = 10.0f,
            GLfloat mouse_sensitivity = 0.1f,
            glm::vec3 position        = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3 up              = glm::vec3(0.0f, 1.0f, 0.0f),
            GLfloat yaw               = glm::radians(-90.0f),
            GLfloat pitch             = 0.0f
    )
        : width(width)
        , height(height)
        , near(near)
        , far(far)
        , fov(fov)
        , speed(speed)
        , mouse_sensitivity(mouse_sensitivity)
        , position(position)
        , up(up)
        , yaw(yaw)
        , pitch(pitch)
    {
        update();
    }

    glm::mat4 get_view_matrix()
    {
        return glm::lookAt(position, position + direction /* target */, up);
    }

    glm::mat4 get_projection_matrix()
    {
        return glm::perspective(fov, width / height, near, far);
    }

    void move_forward(GLfloat deltatime)
    {
        position += direction * speed * deltatime;
    }

    void move_backward(GLfloat deltatime)
    {
        position -= direction * speed * deltatime;
    }

    void move_right(GLfloat deltatime)
    {
        position += right * speed * deltatime;
    }

    void move_left(GLfloat deltatime)
    {
        position -= right * speed * deltatime;
    }

    void process_mouse_movement(GLfloat xoffset, GLfloat yoffset)
    {
        yaw += xoffset * mouse_sensitivity;

        GLfloat new_pitch = pitch - yoffset * mouse_sensitivity;
        if (new_pitch > pitch_max)
            new_pitch = pitch_max;
        else if (new_pitch < pitch_min)
            new_pitch = pitch_min;

        pitch = new_pitch;
        update();
    }
};

#endif /* CAMERA_HPP */
