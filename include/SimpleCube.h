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

    Point3D position;
    int16_t Size;
    Point3D Points[8];

    Vector3D vec3d[12];
};