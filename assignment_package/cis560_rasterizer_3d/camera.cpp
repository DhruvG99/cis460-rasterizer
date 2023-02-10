#include "camera.h"
#include <math.h>
#include <iostream>
# define M_PI           3.14159265358979323846
Camera::Camera()
    : forwardZ(0.0f, 0.0f, -1.0f, 0.0f), rightX(1.0f, 0.0f, 0.0f, 0.0f), upY(0.0f, 1.0f, 0.0f, 0.0f),
      fov(45.0f), camPos(0.0f, 0.0f, 10.0f, 1.0f),
      nearClip(0.01f), farClip(100.0f), aspRatio(1.0f)
{}

glm::mat4 Camera::viewMat()
{
    glm::mat4 viewR(
            glm::vec4(rightX[0], upY[0], forwardZ[0], 0.0f),
            glm::vec4(rightX[1], upY[1], forwardZ[1], 0.0f),
            glm::vec4(rightX[2], upY[2], forwardZ[2], 0.0f),
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
            );

    glm::mat4 viewT(1.0f);
    viewT[3] = glm::vec4(-camPos[0], -camPos[1], -camPos[2], 1.0f);
    return viewR*viewT;
}

glm::mat4 Camera::projMat()
{
    glm::mat4 projM(
                glm::vec4(1.0f/(aspRatio*tan(fov/2.0f)), 0.0f, 0.0f, 0.0f),
                glm::vec4(0.0f, 1.0f/tan(fov/2.0f), 0.0f, 0.0f),
                glm::vec4(0.0f, 0.0f, farClip/(farClip-nearClip), 1.0f),
                glm::vec4(0.0f, 0.0f, -nearClip*farClip/(farClip-nearClip), 0.0f)
                );
    return projM;
}

void Camera::translateZ(float z)
{
    glm::mat4 tz (1.0f);
    tz[3] = z*forwardZ;
    tz[3][3] = 1.0f;
    this->camPos = tz*this->camPos;
}

void Camera::translateX(float x)
{
    glm::mat4 tx (1.0f);
    tx[3] = x*rightX;
    tx[3][3] = 1.0f;
    this->camPos = tx*this->camPos;
}

void Camera::translateY(float y)
{
    glm::mat4 ty (1.0f);
    ty[3] = y*upY;
    ty[3][3] = 1.0f;
    this->camPos = ty*this->camPos;
}

void Camera::rotateZ(float rot)
{
    rot = rot*(M_PI/180.0f);
    glm::vec3 z(forwardZ);
    glm::vec3 x(rightX);
    glm::vec3 y(upY);
    rightX = glm::vec4(x*cos(rot) + glm::cross(z,x)*sin(rot) + z*glm::dot(z,x)*(1-cos(rot)), 0.0f);
    upY = glm::vec4(y*cos(rot) + glm::cross(z,y)*sin(rot) + z*glm::dot(z,y)*(1-cos(rot)), 0.0f);
}

void Camera::rotateX(float rot)
{
    rot = rot*(M_PI/180.0f);
    glm::vec3 z(forwardZ);
    glm::vec3 x(rightX);
    glm::vec3 y(upY);
    forwardZ = glm::vec4(z*cos(rot) + glm::cross(x,z)*sin(rot) + x*glm::dot(x,z)*(1-cos(rot)), 0.0f);
    upY = glm::vec4(y*cos(rot) + glm::cross(x,y)*sin(rot) + x*glm::dot(x,y)*(1-cos(rot)), 0.0f);
}

void Camera::rotateY(float rot)
{
    rot = rot*(M_PI/180.0f);
    glm::vec3 z(forwardZ);
    glm::vec3 x(rightX);
    glm::vec3 y(upY);
    rightX = glm::vec4(x*cos(rot) + glm::cross(y,x)*sin(rot) + y*glm::dot(y,x)*(1-cos(rot)), 0.0f);
    forwardZ = glm::vec4(z*cos(rot) + glm::cross(y,z)*sin(rot) + y*glm::dot(y,z)*(1-cos(rot)), 0.0f);
}

