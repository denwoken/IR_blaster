#ifndef Vectors_H_
#define Vectors_H_
#include "Arduino.h"


typedef struct Point2dF {
    float x;
    float y;
}Point2dF;

typedef union Vector2dF {
    struct {
        Point2dF point0;
        Point2dF point1;
    };
    struct {
        float x0;
        float y0;
        float x1;
        float y1;
    };
}Vector2dF;


typedef struct Point2d {
    uint8_t x;
    uint8_t y;
}Point2d;


#endif // end 