#pragma once
#include "Vectors.h"

class SimpleCube
{
public:
    SimpleCube(int16_t Size);

    void SetDefault();

    void rotateY(float fi);
    void rotateX(float fi);
    void rotateZ(float fi);

    void draw_skeleton();
    void draw();
    void func(Point2D *P, uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    Point3D position;
    int16_t Size;
    Point3D Points[8];

    Vector3D vec3d[12];
};