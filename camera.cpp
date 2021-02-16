#include "camera.hpp"

const GLfloat Camera::pitch_max =  89.0f;
const GLfloat Camera::pitch_min = -89.0f;
const GLfloat Camera::fov_max   =  45.0f;
const GLfloat Camera::fov_min   =  1.0f;


void Camera::update()
{
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    right = glm::normalize(glm::cross(direction, up));
}


void Camera::process_mouse_movement(GLfloat xoffset, GLfloat yoffset)
{
    GLfloat new_pitch = pitch - yoffset * mouse_sensitivity;
    if (new_pitch > Camera::pitch_max)
        new_pitch = Camera::pitch_max;
    else if (new_pitch < Camera::pitch_min)
        new_pitch = Camera::pitch_min;

    pitch = new_pitch;
    yaw += xoffset * mouse_sensitivity;

    update();
}
