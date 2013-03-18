#ifndef BALL_H
#define BALL_H

#include <gdiplus.h>
#include <vector>
#include "Vector2D.h"
//#include "Physics.h"
#include "Force.h"
#include "Window.h"
#include "Matrix3x3.h"

extern Gdiplus::Point TransformToWindow(const Vector2D &);
extern int MetersToPixels(double);

class Ball
{
public:
  friend void Integrate(Ball&, double);
  friend void UpdateForces(Ball&, double);
  friend void ApplyGravity(Ball&);

  // Constructor
  Ball(Window *window);
  // Destructor
  ~Ball();


  // Initializes the ball.
  void Initialize(double mass, double radius,
    Vector2D position);

  Vector2D Position;
  Vector2D Velocity;
  Vector2D Acceleration;
  double Mass;
  double Radius;
  double AngularVelocity;
  double AngularAcceleration;
  double Orientation;

  void Update(double dt);
  void Draw(Gdiplus::Graphics *g);
  void AddForce(Force force);
  void ApplyImpulse(const Vector2D& impulse);
  void ApplyAngularImpulse(const double impulse);

private:
  static int numBalls;

  // All forces acting on the object over an update will be accumulated here.
  std::vector<Force> forces;
  Vector2D forceAccumulator;
  Window *window;
  static Image *ballImg;
};


Image *Ball::ballImg = nullptr;
int Ball::numBalls = 0;

Ball::Ball(Window *window) 
{ 
  Mass = 1.0;
  this->window = window;
  numBalls++;
}

Ball::~Ball() 
{ 
  numBalls--;
  if(numBalls == 0)
  {
    delete ballImg;
  }
}

void Ball::Initialize(double mass, double radius,
                      Vector2D position)
{
  if(Ball::ballImg == nullptr)
  {
    ballImg = new Image(L"ball.png");
  }

  Mass = mass;
  Radius = radius;
  Position = position;
  AngularVelocity = 0;
  AngularAcceleration = 0;
  Orientation = 0;
}

void Ball::Draw(Gdiplus::Graphics *g)
{
  Gdiplus::Point pos = window->TransformToWindow(Position);

  // Calculate the size ratio between the ball image and the balls size.
  double scaleFac = MetersToPixels(Radius * 2) / 64.0;

  g->ScaleTransform(scaleFac, scaleFac, MatrixOrder::MatrixOrderPrepend);
  g->TranslateTransform(MetersToPixels(Radius) / scaleFac,
    MetersToPixels(Radius) / scaleFac);
  g->TranslateTransform(
    (pos.X - MetersToPixels(Radius)) / scaleFac,
    (pos.Y - MetersToPixels(Radius)) / scaleFac);  
  g->RotateTransform(Orientation);
  


  g->DrawImage(
    ballImg,
    -MetersToPixels(Radius) / scaleFac,
    -MetersToPixels(Radius) / scaleFac,
    0,
    0,
    64,
    64,
    Unit::UnitPixel
  );

  g->ResetTransform();
}


inline void Ball::AddForce(Force force)
{
  forces.push_back(force);
}

void Ball::ApplyImpulse(const Vector2D &impulse)
{
  Velocity += impulse * (1.0 /  Mass);
}

void Ball::ApplyAngularImpulse(const double impulse)
{
  AngularVelocity += impulse;
}

#endif