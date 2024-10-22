#include "camera.h"
#include "glm/ext.hpp"

AppCamera::AppCamera()
{
    vFov = 3.14/4.0;
    aspect = 1.77778f;
    nearPlane = 0.1f;
    farPlane = 100.f;
    position = {0.f, 0.f, -5.f};
    forwardVector = {0.f, 0.f, 1.f};
    upwardVector = {0.f, 1.f, 0.f};
}

glm::mat4 AppCamera::getViewMatrix()
{
    return glm::lookAtRH(position, position + forwardVector, upwardVector);
}

glm::mat4 AppCamera::getProjMatrix()
{
    glm::mat4 persp = glm::perspective(vFov, aspect, nearPlane, farPlane);
    persp[1][1] *= -1.0f;
    return persp;
}

glm::vec3 AppCamera::getRightVector()
{
    return glm::cross(forwardVector, upwardVector);
}

glm::vec3 AppCamera::getUpwardVector()
{
    return upwardVector;
}

glm::vec3 AppCamera::getForwardVector()
{
    return forwardVector;
}

float AppCamera::getVFOV()
{
    return vFov;
}

float AppCamera::getAspectRatio()
{
    return aspect;
}

void AppCamera::moveForward(float dist)
{
    position = position + dist * forwardVector;
}

void AppCamera::moveRight(float dist)
{
    position = position + dist * getRightVector();
}

void AppCamera::moveUp(float dist)
{
    position = position + dist * upwardVector;
}

void AppCamera::lookUp(float angle)
{
    glm::mat4 rotationMatrix = glm::rotate(glm::identity<glm::mat4>(), angle, getRightVector());
    upwardVector = rotationMatrix * glm::vec4{upwardVector, 0.f};
    forwardVector = rotationMatrix * glm::vec4{forwardVector, 0.f};
}

void AppCamera::lookRight(float angle)
{
    const glm::vec3 worldUp = {0.f, 1.f, 0.f};
    glm::mat4 rotationMatrix = glm::rotate(glm::identity<glm::mat4>(), -1.f * angle, worldUp);
    upwardVector = rotationMatrix * glm::vec4{upwardVector, 0.f};
    forwardVector = rotationMatrix * glm::vec4{forwardVector, 0.f};
}
