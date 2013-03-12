/* This file contains all the physics and math related calculations
 * for the application. Separation was done in order simplify
 * evaluation for our teacher. For Vector and Matrix specific calculations
 * please refer to "Vector2D.h" and "Matrix2x2.h" respectively. */

#ifndef PHYSICS_H
#define PHYSICS_H

#include <gdiplus.h>
#include <vector>
#include "Vector2D.h"
#include "Ball.h"
#include "Force.h"
#include "Line.h"



// ---------- PHYSICS SETTINGS ------------ //
const double MetersPerPixel = 1.0 / 100.0;
const double GravityCoefficient = 9.82;
const Vector2D GravityDirection(0, -1.0);

// Variables are defined elsewhere
extern const int ScreenWidth;
extern const int ScreenHeight;


// Called to update the physics simulation for a given ball.
// Besides drawing, these are the only things that ever happen to a ball.
void Update(Ball *ball, double delta)
{   
  // Update the forces acting on the ball.
  // Applies them to the accumulator for the ball, and removes expired forces.
  UpdateForces(*ball, delta);

  // Integrate the forces, resulting in acceleration if any.
  Integrate(*ball, delta);

  ball->AngularVelocity *= 0.99;
}


// Used to transform a value in meters into the equivalent in pixels
inline int MetersToPixels(double meters)
{
  return static_cast<int>(meters * (1.0 / MetersPerPixel));
}

void UpdateForces(Ball &ball, double dt)
{
  for(auto it = ball.forces.begin(); it != ball.forces.end(); ++it)
  {

    if(it->Permanent == false)
    {
      it->TimeLeft -= dt;

      // Remove expired forces
      if(it->TimeLeft - dt <= 0.0)
      {
        it = ball.forces.erase(it);
        if(it == ball.forces.end())
          break;

        continue;
      }
    }

    ball.forceAccumulator += it->Direction * it->Magnitude * dt;
    
  }

  ApplyGravity(ball);
}

/* Integrates a balls position by calculating the acting forces,
 * and then integrating over time. */
void Integrate(Ball &ball, double dt)
{
  // Resulting acceleration
  ball.Acceleration = ball.forceAccumulator * (1.0 /  ball.Mass);
  ball.forceAccumulator = Vector2D(0,0);
  // Integrate the balls velocity over time
  ball.Velocity = ball.Acceleration * dt + ball.Velocity;

  // Integrate the position over time based on velocity
  ball.Position = (ball.Acceleration * pow(dt, 2.0)) * (1.0 / 2.0) +
    ball.Velocity * dt + ball.Position;

  ball.AngularVelocity = ball.AngularAcceleration * dt + ball.AngularVelocity;

  ball.Orientation = ball.AngularAcceleration * pow(dt, 2.0) * (1.0 / 2.0) +
    ball.AngularVelocity * dt + ball.Orientation;

  // This can be condensed to the following if we want to store less values
#if 0
  ball.Position = (ball.forceAccumulator *
    (1 /  ball.Mass) * pow(dt, 2)) * (1.0f / 2.0f) + 
    ball.forceAccumulator * (1.0f /  ball.Mass) * dt + ball.Velocity + ball.Position;
#endif
}

void ApplyGravity(Ball &ball)
{
  // We multiply by the balls mass to cancel out the division due to F = ma
  // Gravitation is always constant for all objects.
  Vector2D gravForce(GravityDirection * (ball.Mass * GravityCoefficient));
  ball.forceAccumulator += gravForce;
}

// Determines the closest point on the line made out of l1 and l2.
Vector2D ClosestPointOnLine(const Vector2D &point, const Line &line)
{
  Vector2D lineVec = line.GetStart().X < line.GetEnd().X ?
    line.GetEnd() -line.GetStart() : line.GetStart() - line.GetEnd();
  
  Vector2D diff = line.GetStart() - point;

  float dot = Vector2D::Dot(lineVec.Unit(), diff.Unit());

  float lineSegLength = dot * diff.Length();

  Vector2D nearestPoint = line.GetStart() + lineVec.Unit() * -lineSegLength;

  return nearestPoint;
}

float PointLineDistance(const Vector2D &point, const Line &line)
{
  return (ClosestPointOnLine(point, line) - point).Length();
}



#endif