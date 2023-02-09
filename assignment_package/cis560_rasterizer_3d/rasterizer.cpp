#include "rasterizer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <cmath>

Rasterizer::Rasterizer(const std::vector<Polygon>& polygons)
    :  zdepth(262144,FLT_MAX) , m_polygons(polygons)
{}

float Rasterizer::triangleArea(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3) const
{
    float area = 0.5f*abs(
                p1[0]*p2[1] - p2[0]*p1[1] +
            p2[0]*p3[1] - p3[0]*p2[1] +
            p3[0]*p1[1] - p1[0]*p3[1]
            );
    return area;
}

glm::vec3 Rasterizer::baryInterp(glm::vec4 p, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3) const
{
    glm::vec3 coeff;
    float area = triangleArea(p1, p2, p3);
    coeff[0] = triangleArea(p, p2, p3)/area;
    coeff[1] = triangleArea(p, p1, p3)/area;
    coeff[2] = triangleArea(p, p1, p2)/area;
    return coeff;
}

//to be modified for multiple polygons
void Rasterizer::fillTriangle(unsigned int p, const struct Triangle& t, QImage* img)
{
    Polygon poly = this->m_polygons[p];
    Vertex v1 = poly.m_verts[t.m_indices[0]];
    Vertex v2 = poly.m_verts[t.m_indices[1]];
    Vertex v3 = poly.m_verts[t.m_indices[2]];
    std::cout << "Vertices: " <<std::endl
              << v1.m_pos[0] << ", " << v1.m_pos[1] << std::endl
                 << v2.m_pos[0] << ", " << v2.m_pos[1] << std::endl
                    << v3.m_pos[0] << ", " << v3.m_pos[1] << std::endl;
    //TODO:
    //>world coordinates-pixel conversion
    //triangle conversion to pixel for lineseg
    //>find bounds here
    Lineseg lines[] = {Lineseg(v1.m_pos,v2.m_pos),
                     Lineseg(v2.m_pos,v3.m_pos),
                     Lineseg(v3.m_pos,v1.m_pos)};

    //ensure we only search rows in bounding box
    int y_min = floor(t.ymin);
    int y_max = ceil(t.ymax);
    std::cout << "Y Bounds: " << t.ymin << ", " << t.ymax <<std::endl;
    std::cout << "X Bounds: " << t.xmin << ", " << t.xmax <<std::endl;
    for(int row = y_min; row<y_max; row++)
    {
        float xLeft = 512.0f, xRight = 0.0f;
        for(Lineseg line: lines)
        {
            float x;
            //if intersection found
            if(line.getIntersection(row, &x))
            {
                //clamp at bounding box (may be outside screen)
                if(x<t.xmin || x>t.xmax)
                    continue;
                xLeft = glm::min(xLeft, x);
                xRight = glm::max(xRight, x);
            }
        }
        //clamp at screen boundary
        xLeft = glm::max(glm::min(xLeft, 512.0f),0.0f);
        xRight = glm::min(glm::max(xRight, 0.0f),512.0f);

        //fill between xLeft and xRight
        //dangerous stuff, using float as a counter
        for(float xcoord = xLeft; xcoord<xRight; xcoord++)
        {
            //zdepth[floor(xcoord + 512.0f*row)] = ;
            float zp = v1.m_pos[2];
            if(zp < this->zdepth[floor(xcoord + 512.0f*row)])
            {
                zdepth[floor(xcoord + 512.0f*row)] = zp;
                glm::vec4 v(xcoord, static_cast<float>(row), 0.0f, 1.0f);
                glm::vec3 coeff = baryInterp(v ,v1.m_pos, v2.m_pos, v3.m_pos);
                glm::vec3 cvec = coeff[0]*v1.m_color + coeff[1]*v2.m_color + coeff[2]*v3.m_color;
                QRgb cval = qRgb(cvec[0],cvec[1],cvec[2]);
                img->setPixel(xcoord, row, cval);
            }
        }
    }

}

void Rasterizer::fillPolygon(QImage* img)
{

    for(unsigned int i = 0; i<this->m_polygons.size(); i++)
    {
        Polygon p = m_polygons[i];
        std::cout << p.m_name.toStdString() << std::endl;
        for(unsigned int j=0; j<p.m_tris.size(); j++)
        {
            std::cout << "Triangle No.: " << j << std::endl;
            fillTriangle(i, p.m_tris[j], img);
        }
    }
}

QImage Rasterizer::RenderScene()
{
    QImage result(512, 512, QImage::Format_RGB32);
    // Fill the image with black pixels.
    // Note that qRgb creates a QColor,
    // and takes in values [0, 255] rather than [0, 1].
    result.fill(qRgb(0.f, 0.f, 0.f));
    // TODO: Complete the various components of code that make up this function.
    // It should return the rasterized image of the current scene.
    fillPolygon(&result);
//    std::cout << "Ended RenderScene"<<std::endl;
    // Make liberal use of helper functions; writing your rasterizer as one
    // long RenderScene function will make it (a) hard to debug and
    // (b) hard to write without copy-pasting. Also, Adam will be sad when
    // he reads your code.

    // Also! As per the style requirements for this assignment, make sure you
    // use std::arrays to store things like your line segments, Triangles, and
    // vertex coordinates. This lets you easily use loops to perform operations
    // on your scene components, rather than copy-pasting operations three times
    // each!
    return result;
}

void Rasterizer::ClearScene()
{
    m_polygons.clear();
}
