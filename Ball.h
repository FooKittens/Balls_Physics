#ifndef BALL_H
#define BALL_H

#include <gdiplus.h>
#include <vector>
#include "Vector2D.h"
//#include "Physics.h"
#include "Force.h"

class Ball
{
public:
  friend void Integrate(Ball&, double);
  friend void UpdateForces(Ball&, double);
  friend void ApplyGravity(Ball&);

  // Constructor
  Ball();
  // Destructor
  ~Ball();

  Vector2D Position;
  Vector2D Velocity;
  Vector2D Acceleration;
  float Mass;

  void Update(double dt);
  void Draw(Gdiplus::Graphics *g);
  void AddForce(Force force);

private:
  // All forces acting on the object over an update will be accumulated here.
  std::vector<Force> forces;
  Vector2D forceAccumulator;
};


Ball::Ball() { Mass = 1.0f; }

Ball::~Ball() { }


inline void Ball::AddForce(Force force)
{
  forces.push_back(force);
}

#endif