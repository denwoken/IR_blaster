#pragma once

#include "Arduino.h"

typedef struct
{
  float alpha;
  float betta;
  float gamma;
} Normal;

class Point2D
{
public:
  Point2D(){};
  Point2D(int16_t x, int16_t y)
  {
    this->x = x;
    this->y = y;
  };
  int16_t x, y;
  Point2D operator/(int16_t b)
  {
    Point2D result(*this);
    result /= b;
    return result;
  }
  void operator/=(int16_t b)
  {
    this->x /= b;
    this->y /= b;
  }

  Point2D operator*(int16_t b)
  {
    Point2D result(*this);
    result *= b;
    return result;
  }
  void operator*=(int16_t b)
  {
    this->x *= b;
    this->y *= b;
  }

  Point2D operator-(Point2D const &b)
  {
    Point2D result(*this);
    result -= b;
    return result;
  }
  void operator-=(Point2D const &b)
  {
    this->x -= b.x;
    this->y -= b.y;
  }

  Point2D operator+(Point2D const &b)
  {
    Point2D result(*this);
    result += b;
    return result;
  }
  void operator+=(Point2D const &b)
  {
    this->x += b.x;
    this->y += b.y;
  }

  void operator=(Point2D const &b)
  {
    this->x = b.x;
    this->y = b.y;
  }

  bool operator==(Point2D const &b)
  {
    return (this->x == b.x) && (this->y == b.y);
  }
  bool operator!=(Point2D const &b)
  {
    return (this->x != b.x) && (this->y != b.y);
  }
};

class Point3D
{
public:
  Point3D(){};
  Point3D(int16_t x, int16_t y, int16_t z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  };
  int16_t x, y, z;
  Point3D operator/(int16_t b)
  {
    Point3D result(*this);
    result /= b;
    return result;
  }
  void operator/=(int16_t b)
  {
    this->x /= b;
    this->y /= b;
    this->z /= b;
  }

  Point3D operator*(int16_t b)
  {
    Point3D result(*this);
    result *= b;
    return result;
  }
  void operator*=(int16_t b)
  {
    this->x *= b;
    this->y *= b;
    this->z *= b;
  }

  Point3D operator-(Point3D const &b)
  {
    Point3D result(*this);
    result -= b;
    return result;
  }
  void operator-=(Point3D const &b)
  {
    this->x -= b.x;
    this->y -= b.y;
    this->z -= b.z;
  }

  Point3D operator+(Point3D const &b)
  {
    Point3D result(*this);
    result += b;
    return result;
  }
  void operator+=(Point3D const &b)
  {
    this->x += b.x;
    this->y += b.y;
    this->z += b.z;
  }

  void operator=(Point3D const &b)
  {
    this->x = b.x;
    this->y = b.y;
    this->z = b.z;
  }

  bool operator==(Point3D const &b)
  {
    return (this->x == b.x) && (this->y == b.y) && (this->z == b.z);
  }
  bool operator!=(Point3D const &b)
  {
    return (this->x != b.x) && (this->y != b.y) && (this->z != b.z);
  }
};

// union Vector2D
// {
//   struct point
//   {
//     Point2D _0;
//     Point2D _1;
//   };
//   struct
//   {
//     Point2D *ptr_point0;
//     Point2D *ptr_point1;
//   };
//   struct
//   {
//     int16_t x0, y0;
//     int16_t x1, y1;
//   };
// };

class Vector2D
{
public:
  Vector2D(){};

  union
  {
    Point2D point0;
    Point2D *ptr_point0;
    struct
    {
      int16_t x0, y0;
    };
  };
  union
  {
    Point2D point1;
    Point2D *ptr_point1;
    struct
    {
      int16_t x1, y1;
    };
  };
};

class Vector3D
{
public:
  Vector3D(){};

  union
  {
    Point3D point0;
    Point3D *ptr_point0;
    struct
    {
      int16_t x0, y0, z0;
    };
  };
  union
  {
    Point3D point1;
    Point3D *ptr_point1;
    struct
    {
      int16_t x1, y1, z1;
    };
  };
};

void RotateY(Point3D *point3D, float fi);
void RotateX(Point3D *point3D, float fi);
void RotateZ(Point3D *point3D, float fi);

void Rotate(Point2D *point2D, float fi);

Point2D Simple_projection(Point3D *point3D);
