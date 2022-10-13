#include "SimpleCube.h"
#include "Vectors.h"

//#include "display_drivers/driver_ST7735.h"
#include "Graphics.h"

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
void SimpleCube::draw_skeleton()
{
  Vector2D vec2d;
  for (uint8_t i = 0; i < 12; i++)
  {
    vec2d.point0 = Simple_projection(vec3d[i].ptr_point0);
    vec2d.point1 = Simple_projection(vec3d[i].ptr_point1);
    Serial.printf("\n");
    tft.writeLine(vec2d.x0, vec2d.y0, vec2d.x1, vec2d.y1, i < 3 ? RED : GREEN);
  }
}

Point3D normal(Point3D p0, Point3D a, Point3D b)
{
  Point3D result;
  a = a - p0;
  b = b - p0;
  result.x = a.y * b.z - b.y * a.z;
  result.y = -a.x * b.z + b.x * a.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}
float scal_product(Point3D a, Point3D b)
{
  return (a.x * b.x + a.y * b.y + a.z * b.z) / (a.len() * b.len());
}

void SimpleCube::func(Point2D *P, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
  Point3D cam_pos = {0, 0, -300};
  Point3D n, cam;
  n = normal(Points[a], Points[c], Points[d]);
  cam = Points[a] - cam_pos;
  float cos = 0.7 * scal_product(n, cam);
  //   Serial.printf("P\tx=%d\ty=%d\n", P[0].x, P[0].y);
  //   Serial.printf("Point\tx=%d\ty=%d\tz=%d\n", Points[0].x, Points[0].y, Points[0].z);
  //   Serial.printf("cam\tdx=%d\tdy=%d\tdz=%d\n", cam.x, cam.y, cam.z);
  //   Serial.printf("normal\tdx=%d\tdy=%d\tdz=%d\n", n.x, n.y, n.z);
  //   Serial.printf("cos*256\t=%f\n", cos);

  // Point3D pp = Points[0] + n / 500;
  // Point2D xx = Simple_projection(&pp);
  // tft.writeLine(P[0].x, P[0].y, xx.x, xx.y, RED);
  // tft.writePixel(P[0].x, P[0].y, WHITE);
  // Serial.printf("\n");

  // Point2D ww = Simple_projection(&cam_pos);
  // tft.writeLine(P[0].x, P[0].y, ww.x, ww.y, WHITE);
  if (cos < 0)
  {
    uint16_t col = Graphics::gray8_to_color565((uint8_t)(-256 * (-0.2 + cos)));
    // tft.fillTriangle(P[0].x, P[0].y, P[1].x, P[1].y, P[3].x, P[3].y, c);
    // tft.fillTriangle(P[2].x, P[2].y, P[1].x, P[1].y, P[3].x, P[3].y, c);

    tft.fillTriangle(P[a].x, P[a].y, P[c].x, P[c].y, P[d].x, P[d].y, col);
    tft.fillTriangle(P[b].x, P[b].y, P[d].x, P[d].y, P[c].x, P[c].y, col);
  }
}
void SimpleCube::draw()
{
  Point2D P[8];
  for (uint8_t i = 0; i < 8; i++)
    P[i] = Simple_projection(Points + i);

  func(P, 0, 2, 1, 3);
  func(P, 6, 1, 2, 5);
  func(P, 5, 0, 1, 4);
  func(P, 5, 7, 4, 6);
  func(P, 6, 3, 7, 2);
  func(P, 7, 0, 4, 3);

  // func(P[0], P[1], P[2]);
  // func(P[0], P[3], P[2]);

  // func(P[0], P[4], P[7]);
  // func(P[0], P[3], P[7]);

  // func(P[2], P[1], P[5]);
  // func(P[2], P[6], P[5]);

  // func(P[3], P[2], P[6]);
  // func(P[3], P[7], P[6]);

  // func(P[7], P[4], P[5]);
  // func(P[7], P[6], P[5]);
}