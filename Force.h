#ifndef FORCE_H
#define FORCE_H
#include "Vector2D.h"

struct Force
{
  Force(const Vector2D &direction, float mag, float duration)
  {
    Direction = direction;
    Magnitude = mag;
    TimeLeft = duration;
    Permanent = false;
  }

  Vector2D Direction;
  float Magnitude;
  float TimeLeft;
  bool Permanent;
};

#endif