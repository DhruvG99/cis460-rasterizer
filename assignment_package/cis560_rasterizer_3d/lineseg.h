#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <QString>
#include <QImage>
#include <QColor>

class Lineseg
{
public:
    //vectors storing segment's endpoints
    glm::vec4 p1;
    glm::vec4 p2;

    Lineseg();
    Lineseg(glm::vec4 pt1, glm::vec4 pt2);
    bool getIntersection(int y, float* x) const;
};

