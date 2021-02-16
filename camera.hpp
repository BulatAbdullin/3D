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
    glm::vec3 position;
    GLfloat near;
    GLfloat far;
    GLfloat fov; /* field of view */

    GLfloat speed;
    GLfloat mouse_sensitivity;

    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 right;

    GLfloat yaw;
    GLfloat pitch;

    const static GLfloat pitch_max;
    const static GLfloat pitch_min;
    const static GLfloat fov_max;
    const static GLfloat fov_min;

public:
    Camera(GLfloat width,
           GLfloat height,
           glm::vec3 position        = glm::vec3(0.0f, 0.0f, 0.0f),
           GLfloat near              = 0.1f,
           GLfloat far               = 100.0f,
           GLfloat fov               = 45.0f,
           GLfloat speed             = 9.0f,
           GLfloat mouse_sensitivity = 0.1f,
           glm::vec3 up              = glm::vec3(0.0f, 1.0f, 0.0f),
           GLfloat yaw               = -90.0f,
           GLfloat pitch             = 0.0f)
        : width(width)
        , height(height)
        , position(position)
        , near(near)
        , far(far)
        , fov(fov)
        , speed(speed)
        , mouse_sensitivity(mouse_sensitivity)
        , up(up)
        , yaw(yaw)
        , pitch(pitch)
    {
        this->update();
    }

    void update();

    inline glm::vec3 get_position() const
    {
        return position;
    }

    inline glm::mat4 get_view_matrix() const
    {
        return glm::lookAt(position, position + direction /* target */, up);
    }

    inline glm::mat4 get_projection_matrix() const
    {
        return glm::perspective(fov, width / height, near, far);
    }

    inline void move_forward(GLfloat deltatime)
    {
        position += direction * speed * deltatime;
    }

    inline void move_backward(GLfloat deltatime)
    {
        position -= direction * speed * deltatime;
    }

    inline void move_right(GLfloat deltatime)
    {
        position += right * speed * deltatime;
    }

    inline void move_left(GLfloat deltatime)
    {
        position -= right * speed * deltatime;
    }

    inline void move_up(GLfloat deltatime)
    {
        position += up * speed * deltatime;
    }

    inline void move_down(GLfloat deltatime)
    {
        position -= up * speed * deltatime;
    }

    void process_mouse_movement(GLfloat xoffset, GLfloat yoffset);
};

#endif /* CAMERA_HPP */
