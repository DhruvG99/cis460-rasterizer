#pragma once
#include <polygon.h>
#include<lineseg.h>
#include <QImage>

class Rasterizer
{
private:
    //This is the set of Polygons loaded from a JSON scene file
    float zdepth[512*512];
    std::vector<Polygon> m_polygons;
public:
    Rasterizer(const std::vector<Polygon>& polygons);
    float triangleArea(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3) const;
    glm::vec3 baryInterp(glm::vec4 p, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3) const;
    //to fill a row
//    void fillRow(float y_pos, float xl, float xr, Vertex v[3], QImage* img, QRgb val) const;
    //To color a triangle
    void fillTriangle(unsigned int p, const struct Triangle& t, QImage* img) const;
    //To color a polygon
    void fillPolygon(QImage* img);
    QImage RenderScene();
    void ClearScene();
};

/*
 * Flowchart for rasterizer
 * Iterate through all polygons: p
 * Iterate through the triangles in polygon p: t
 * Use bounding box for t
 * Iterate through rows of rasterizer
 * Look for intersection between horizontal and edges
 * Fill out triangle
 * */
