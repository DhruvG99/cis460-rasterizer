#include "Lineseg.h"

Lineseg::Lineseg()
    :p1(),p2()
{

}

Lineseg::Lineseg(glm::vec4 pt1, glm::vec4 pt2)
    : p1(pt1[0],pt1[1],0.0f,1.0f),
      p2(pt2[0],pt2[1],0.0f,1.0f)
{

}

bool Lineseg::getIntersection(int y, float* x) const
{
    float yf =static_cast<float>(y);
    if(this->p1[0] == this->p2[0]) // vertical edge
    {
        if((yf<=p1[1] && yf>=p2[1]) || (yf>=p1[1] && yf<=p2[1]))
        {
            *x = p1[0];
            return true;
        }
    }
    //TODO: y == seg1.y == seg2.y
    else if(this->p1[1] == this->p2[1]) // horizontal edge
        return false; //since the other  two edges will return an intersection
    else //all other cases - oblique
    {
        float yf =static_cast<float>(y);
        if((yf<=p1[1] && yf>=p2[1]) || (yf>=p1[1] && yf<=p2[1]))
        {
            float slope = (this->p1[1] - this->p2[1])/(this->p1[0] - this->p2[0]);
            *x = ((yf-p1[1])/slope) + p1[0];
            return true;
        }
    }

    return false;
}


