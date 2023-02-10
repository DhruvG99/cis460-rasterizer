#include "rasterizer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <cmath>

Rasterizer::Rasterizer(const std::vector<Polygon>& polygons)
    :  zdepth(262144,FLT_MAX) , m_polygons(polygons), c()
{
    //Done while initalizing the rasterizer
    WorldToPixel();
}

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
        //to avoid black lines
        for(float xcoord = xLeft; xcoord<xRight; xcoord++)
        {
            //Current pixel
            glm::vec4 v(xcoord, static_cast<float>(row), 0.0f, 1.0f);
            //barycentric coefficients
            glm::vec3 coeff = baryInterp(v ,v1.m_pos, v2.m_pos, v3.m_pos);

            glm::vec4 v1_world = PixelToCamera(v1.m_pos);
            glm::vec4 v2_world = PixelToCamera(v2.m_pos);
            glm::vec4 v3_world = PixelToCamera(v3.m_pos);
            //interpolated z coordinate
            float zlerp = 1.0f/((coeff[0]/v1_world[2]) + (coeff[0]/v2_world[2]) + (coeff[0]/v3_world[2]));
            //interpolating for uv coordinate
            glm::vec2 uv1 = v1.m_uv;
            glm::vec2 uv2 = v2.m_uv;
            glm::vec2 uv3 = v3.m_uv;
            glm::vec2 uv_p = uv1*coeff[0] + uv2*coeff[1] + uv3*coeff[2];
            //using interpd uv coord to get texture
            glm::vec3 cvec = GetImageColor(uv_p, poly.mp_texture);

            //z-buffering. worked for 2-D but not so sure about 3D
            if(zlerp < this->zdepth[floor(xcoord + 512.0f*row)])
            {
                zdepth[floor(xcoord + 512.0f*row)] = zlerp;
                QRgb cval = qRgb(cvec[0],cvec[1],cvec[2]);
                img->setPixel(xcoord, row, cval);
            }
        }
    }

}

void Rasterizer::WorldToPixel()
{
    for(Polygon& p: this->m_polygons)
    {
        for(unsigned int v=0; v<p.m_verts.size(); v++)
        {
            Vertex vert = p.m_verts[v];
            vert.m_pos = c.projMat()*c.viewMat()*vert.m_pos;
            vert.m_pos = vert.m_pos/vert.m_pos[3];
            vert.m_pos[0] = 0.5f*(vert.m_pos[0]+1.0f)*512.0f;
            vert.m_pos[1] = 0.5f*(1.0f-vert.m_pos[1])*512.0f;

            p.m_verts[v] = vert;
        }
    }
}

void Rasterizer::PixelToWorld()
{
    for(Polygon& p: this->m_polygons)
    {
        for(unsigned int v=0; v<p.m_verts.size(); v++)
        {
            Vertex vert = p.m_verts[v];
            vert.m_pos[0] = 2.0f*(vert.m_pos[0]/512.0f) - 1.0f;
            vert.m_pos[1] = 1.0f - (vert.m_pos[1]/512.0f)*2.0f;
            vert.m_pos = vert.m_pos*vert.m_pos[3];
            vert.m_pos = glm::inverse(c.viewMat())*glm::inverse(c.projMat())*vert.m_pos;
            p.m_verts[v] = vert;
        }
    }
}

glm::vec4 Rasterizer::PixelToCamera(glm::vec4 v)
{
    v[0] = 2.0f*(v[0]/512.0f) - 1.0f;
    v[1] = 1.0f - (v[1]/512.0f)*2.0f;
    v = v*v[3];
    v = glm::inverse(c.projMat())*v;
    return v;
}

void Rasterizer::fillPolygon(QImage* img)
{
    for(unsigned int i = 0; i<this->m_polygons.size(); i++)
    {
        Polygon p = m_polygons[i];
        std::cout << p.m_name.toStdString() << ": " <<
                     p.m_verts.size() <<", " <<
                     p.m_tris.size() << std::endl;
        for(unsigned int j=0; j<p.m_tris.size(); j++)
        {
            fillTriangle(i, p.m_tris[j], img);
        }
    }
}

QImage Rasterizer::RenderScene()
{
    QImage result(512, 512, QImage::Format_RGB32);
    result.fill(qRgb(0.f, 0.f, 0.f));
    //to fix z-buffering issues when using key-presses. need to reinitialize
    zdepth = std::vector(262144,FLT_MAX);
    //main function call to trigger rasterization
    fillPolygon(&result);
    return result;
}

void Rasterizer::ClearScene()
{
    m_polygons.clear();
}
