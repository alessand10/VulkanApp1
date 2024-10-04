#pragma once
#include "math-utilities.h"

class AppCamera {
private:
    glm::vec3 position;
    glm::vec3 forwardVector;
    glm::vec3 upwardVector;
    float vFov;
    float aspect;
    float nearPlane;
    float farPlane;

public:
    AppCamera();
    glm::mat4 getViewMatrix();
    glm::mat4 getProjMatrix();
    glm::vec3 getRightVector();
    glm::vec3 getUpwardVector();
    glm::vec3 getForwardVector();
    float getVFOV();
    float getAspectRatio();
    void moveForward(float dist);
    void moveRight(float dist);
    void moveUp(float dist);
    void lookUp(float angle);
    void lookRight(float angle);
};