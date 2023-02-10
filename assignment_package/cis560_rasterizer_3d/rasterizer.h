#pragma once
#include <polygon.h>
#include <lineseg.h>
#include <camera.h>
#include <QImage>

class Rasterizer
{
private:
    //This is the set of Polygons loaded from a JSON scene file
    //stores z-coordinate for z-buffering
    std::vector<float> zdepth;
    std::vector<Polygon> m_polygons;
public:
    //the camera object
    Camera c;
    Rasterizer(const std::vector<Polygon>& polygons);
    //computes triangle area, ignoring z coordinate
    float triangleArea(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3) const;
    //returns (S1/S,S2/S,S3/S)
    glm::vec3 baryInterp(glm::vec4 p, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3) const;
    //calculates and re-assigns bounds because of world to pixel conversion
    void findBounds(glm::vec4 v1, glm::vec4 v2, glm::vec4 v3, struct Triangle& t);
    //To color a triangle
    void fillTriangle(unsigned int p, struct Triangle& t, QImage* img);
    //Used to convert all vertices of the polygon to World Coordinates
    void PixelToWorld();
    //Used to convert all vertices of the polygon to Pixel Coordinates
    void WorldToPixel();
    //Used to calculate Camera coordinates for barycentric interp, single vertex
    glm::vec4 PixelToCamera(glm::vec4);
    //To color a polygon
    void fillPolygon(QImage* img);
    QImage RenderScene();
    void ClearScene();
};

