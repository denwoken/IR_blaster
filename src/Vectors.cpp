#include "Vectors.h"
#include "Arduino.h"
//#define cos
//#define sin
#include "display_drivers/driver_ST7735.h"
extern uint8_t _width;
extern uint8_t _height;

void RotateY(Point3D *point3D, float fi)
{
    float x = point3D->x;
    float z = point3D->z;
    point3D->x = (cos(fi) * x - sin(fi) * z);
    point3D->z = (sin(fi) * x + cos(fi) * z);
}
void RotateX(Point3D *point3D, float fi)
{
    float y = point3D->y;
    float z = point3D->z;
    point3D->y = cos(fi) * y - sin(fi) * z;
    point3D->z = sin(fi) * y + cos(fi) * z;
}
void RotateZ(Point3D *point3D, float fi)
{
    float x = point3D->x;
    float y = point3D->y;
    point3D->x = cos(fi) * x - sin(fi) * y;
    point3D->y = sin(fi) * x + cos(fi) * y;
}

void Rotate(Point2D *point2D, float fi)
{
    float x = point2D->x;
    float y = point2D->y;
    point2D->x = cos(fi) * x - sin(fi) * y;
    point2D->y = sin(fi) * x + cos(fi) * y;
}

Point2D Simple_projection(Point3D *point3D)
{
    int16_t Xe = point3D->x;
    int16_t Ye = point3D->y;
    int16_t Ze = point3D->z;

    int16_t Xp = _width / 2;
    int16_t Yp = _height / 2;

    // Xp += (10 - Ze) * Xe / (-200 - Ze);
    // Yp += -(10 - Ze) * Ye / (-200 - Ze);
    Xp += (100) * Xe / (Ze + 301);
    Yp += -(100) * Ye / (Ze + 301);

    // Xp += -1 * (camera_Z_near_plain - Ze) * Xe / Ze;
    // Yp += +1 * (camera_Z_near_plain - Ze) * Ye / Ze;
    //  Xp += -Zp * Xe / (Ze - camera_position);
    //  Yp += +Zp * Ye / (Ze - camera_position);

    return {Xp, Yp};
}
/*
Point2D projection(Point3D* point3D) {
    int16_t Xe = point3D->x;
    int16_t Ye = point3D->y;
    int16_t Ze = point3D->z;

    int16_t Xp = WIDTH / 2;
    int16_t Yp = HEIGHT / 2;
    int16_t Zp = camera->position.z + camera->Z_near_plain;

    float t = (Zp - Ze) / (camera->position.z - Ze);
    Xp += Xe + t * (camera->position.x - Xe);
    Yp += Ye + t * (camera->position.y - Ye);

    return { Xp ,Yp };
}*/