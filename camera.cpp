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
    up = glm::normalize(glm::cross(right, direction));
}
