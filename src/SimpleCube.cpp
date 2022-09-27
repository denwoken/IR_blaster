#include "SimpleCube.h"
#include "Vectors.h"

#include "ST7735.h"
#include "Graphics.h"
extern Graphics tft;
extern uint8_t _width;
extern uint8_t _height;

SimpleCube::SimpleCube(int16_t Size)
{
  this->Size = Size;
  SetDefault();

  vec3d[0].ptr_point0 = Points + 0;
  vec3d[0].ptr_point1 = Points + 1;

  vec3d[1].ptr_point0 = Points + 0;
  vec3d[1].ptr_point1 = Points + 3;

  vec3d[2].ptr_point0 = Points + 0;
  vec3d[2].ptr_point1 = Points + 4;

  vec3d[3].ptr_point0 = Points + 6;
  vec3d[3].ptr_point1 = Points + 7;

  vec3d[4].ptr_point0 = Points + 6;
  vec3d[4].ptr_point1 = Points + 2;

  vec3d[5].ptr_point0 = Points + 6;
  vec3d[5].ptr_point1 = Points + 5;

  vec3d[6].ptr_point0 = Points + 3;
  vec3d[6].ptr_point1 = Points + 7;

  vec3d[7].ptr_point0 = Points + 3;
  vec3d[7].ptr_point1 = Points + 2;

  vec3d[8].ptr_point0 = Points + 1;
  vec3d[8].ptr_point1 = Points + 5;

  vec3d[9].ptr_point0 = Points + 1;
  vec3d[9].ptr_point1 = Points + 2;

  vec3d[10].ptr_point0 = Points + 4;
  vec3d[10].ptr_point1 = Points + 7;

  vec3d[11].ptr_point0 = Points + 4;
  vec3d[11].ptr_point1 = Points + 5;
};

void SimpleCube::SetDefault()
{
  Point3D c[8] = {
      {-Size, -Size, -Size},
      {Size, -Size, -Size},
      {Size, -Size, Size},
      {-Size, -Size, Size},

      {-Size, Size, -Size},
      {Size, Size, -Size},
      {Size, Size, Size},
      {-Size, Size, Size}};

  for (uint8_t i = 0; i < 8; i++)
    Points[i] = c[i];
}

void SimpleCube::rotateX(float fi)
{
  for (uint8_t i = 0; i < 8; i++)
    RotateX(Points + i, fi);
}
void SimpleCube::rotateY(float fi)
{
  for (uint8_t i = 0; i < 8; i++)
    RotateY(Points + i, fi);
}
void SimpleCube::rotateZ(float fi)
{
  for (uint8_t i = 0; i < 8; i++)
    RotateZ(Points + i, fi);
}
void SimpleCube::draw()
{
  Vector2D vec2d;
  for (uint8_t i = 0; i < 12; i++)
  {
    vec2d.point0 = Simple_projection(vec3d[i].ptr_point0);
    vec2d.point1 = Simple_projection(vec3d[i].ptr_point1);
    tft.writeLine(vec2d.x0, vec2d.y0, vec2d.x1, vec2d.y1, GREEN);
  }
}