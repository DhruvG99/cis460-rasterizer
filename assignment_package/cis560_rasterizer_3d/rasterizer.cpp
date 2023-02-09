#include "rasterizer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <cmath>
#include <camera.h>

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

void Rasterizer::findBounds(glm::vec4 v1, glm::vec4 v2, glm::vec4 v3, struct Triangle& t)
{
    float xmin = glm::min(v1[0],glm::min(v2[0],v3[0]));
    float ymin = glm::min(v1[1],glm::min(v2[1],v3[1]));
    float xmax = glm::max(v1[0],glm::max(v2[0],v3[0]));
    float ymax = glm::max(v1[1],glm::max(v2[1],v3[1]));
    t.xmin = xmin;
    t.xmax = xmax;
    t.ymin = ymin>0.0f ? ymin:0.0f;
    t.ymax = ymax<512.0f ?  ymax:511.0f;
}

void Rasterizer::fillTriangle(unsigned int p, struct Triangle& t, QImage* img)
{
    Polygon poly = this->m_polygons[p];
    Vertex v1 = poly.m_verts[t.m_indices[0]];
    Vertex v2 = poly.m_verts[t.m_indices[1]];
    Vertex v3 = poly.m_verts[t.m_indices[2]];
    findBounds(v1.m_pos,v2.m_pos,v3.m_pos,t);
//    std::cout<<"Vertex: "<<v1.m_pos[0]<<", "<<v1.m_pos[1]<<", "<<v1.m_pos[2]<<std::endl;
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
    std::cout<<"X Bounds: " << t.xmin << ", " << t.xmax << std::endl <<
               "Y Bounds: " << t.ymin << ", " << t.ymax << std::endl;
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
            img->setPixel(xcoord, row, qRgb(255.0f,255.0f,255.0f));
//            float zp = v1.m_pos[2];
//            if(zp < this->zdepth[floor(xcoord + 512.0f*row)])
//            {
//                zdepth[floor(xcoord + 512.0f*row)] = zp;
//                glm::vec4 v(xcoord, static_cast<float>(row), 0.0f, 1.0f);
//                glm::vec3 coeff = baryInterp(v ,v1.m_pos, v2.m_pos, v3.m_pos);
//                glm::vec3 cvec = coeff[0]*v1.m_color + coeff[1]*v2.m_color + coeff[2]*v3.m_color;
//                QRgb cval = qRgb(cvec[0],cvec[1],cvec[2]);
//                img->setPixel(xcoord, row, cval);
//            }
        }
    }

}

void Rasterizer::fillPolygon(QImage* img)
{
    Camera c;
    for(unsigned int i = 0; i<this->m_polygons.size(); i++)
    {
        Polygon p = m_polygons[i];
        std::cout << p.m_name.toStdString() << ": " <<
                     p.m_verts.size() <<", " <<
                     p.m_tris.size() << std::endl;

        for(unsigned int v=0; v<p.m_verts.size(); v++)
        {
            Vertex vert = p.m_verts[v];
            vert.m_pos = c.projMat()*c.viewMat()*vert.m_pos;
            vert.m_pos = vert.m_pos/vert.m_pos[3];
            vert.m_pos[0] = 0.5*(vert.m_pos[0]+1.0f)*512.0f;
            vert.m_pos[1] = 0.5*(1.0f-vert.m_pos[1])*512.0f;

            m_polygons[i].m_verts[v] = vert;
            std::cout<<"Vertex: "<<vert.m_pos[0]<<", "<<vert.m_pos[1]<<", "<<vert.m_pos[2]<<std::endl;
        }
        for(unsigned int j=0; j<p.m_tris.size(); j++)
        {
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
