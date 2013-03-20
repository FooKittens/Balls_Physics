#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>
#include <gdiplus.h>
#include <vector>
#include "GameTimer.h"
#include "Matrix3x3.h"
#include "Vector2D.h"

using namespace Gdiplus;

// Prototype.
class Ball;
class Line;

class Window
{
public:
  // Constructor
  Window(HINSTANCE instance, UINT width, UINT height);

  // Destructor
  ~Window();

  // Create and initialize the window
  bool Initialize();

  // Displays the window and starts the message loop.
  bool Run();

  // Transforms a vector into screen space.
  Gdiplus::Point TransformToWindow(const Vector2D &vec) const;

  LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
private:
  // Needs to be static due to memberfunction pointers being dumb.
  static LRESULT CALLBACK StaticWinProc(HWND, UINT, WPARAM, LPARAM);
  
  // Updates the physics
  void UpdateSimulation(double);

  // Called to draw the state of the physics.
  void Draw();

  void GetFpsString(WCHAR *buffer, int size);

  void UpdateGlobalRestitution();
  void ResetBalls();
  void DoBallCollisions(Ball *pBall);

  // ---- "Game" Variables ---- //
  // Timer used for precision timing.
  GameTimer timer;
  std::vector<Ball *> balls;
  std::vector<Line *> lines;

  // ---- Window variables ---- //
  HINSTANCE appInstance;
  HWND hWindow;
  UINT width, height;
  double globalRestitution;
  bool ballCollisionsOn;
  
  // ---- Graphics and GDI ---- //
  Graphics *bufferGraphics;
  Graphics *windowGraphics;
  ULONG gdiStartToken;
  Bitmap *backBuffer;
  Matrix3x3 screenTransformMat;
  int frames;
  int lastFps;
  WCHAR *fpsStrBuffer;
  Font *fpsFont;
  double frameTimer;
};

#endif