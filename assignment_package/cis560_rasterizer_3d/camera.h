#pragma once
#include <glm/glm.hpp>
#include <vector>

class Camera
{
private:
    glm::vec4 forwardZ;
    glm::vec4 rightX;
    glm::vec4 upY;
    float fov;
    glm::vec4 camPos;
    float nearClip;
    float farClip;
    float aspRatio;
public:
    Camera();
    glm::mat4 viewMat();
    glm::mat4 projMat();
    void translateZ(float);
    void translateX(float);
    void translateY(float);
    void rotateZ(float);
    void rotateX(float);
    void rotateY(float);
};

