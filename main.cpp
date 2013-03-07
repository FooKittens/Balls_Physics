#include <Windows.h>
#include <gdiplus.h>
#include "GameTimer.h"
#include "Vector2D.h"
#include "Ball.h"
#include "Physics.h"

#include <vector>

#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Ws2_32.lib")

using namespace Gdiplus;
using std::vector;

/// ------- SETTINGS --------- ///
const int ScreenWidth = 800;
const int ScreenHeight = 640;


// Transformation matrix used to transform our world units into screen coordinates.
Matrix3x3 ScreenTransform;


HWND hwnd;
ULONG gdiStartToken;

// Buffer we use to draw on to prevent screen flickering
Bitmap *backBuffer;

GameTimer timer;

vector<Ball *> balls;

// Procedure called by windows to tell the application to process events.
LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

// Will be called to initialize the application
void Init();

// Updates physics
void UpdateSimulation(double delta);

// Draws a representation of the physics state.
void Draw();

// Called when the program is closing to cleanup all our resources.
void Cleanup();

int WINAPI WinMain(HINSTANCE inst, HINSTANCE pinst, TCHAR *cmd, int cmdshow)
{
  const TCHAR *appClass = TEXT("BallClass");

  WNDCLASS wclass;
  wclass.cbClsExtra = 0;
  wclass.cbWndExtra = 0;
  wclass.hbrBackground = 0;
  wclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wclass.hInstance = inst;
  wclass.lpfnWndProc = &WinProc;
  wclass.lpszClassName = appClass;
  wclass.lpszMenuName = NULL;
  wclass.style = 0;


  RegisterClass(&wclass);

  hwnd = CreateWindow(
    appClass,
    "Physics Simulation",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    ScreenWidth + 8 * 2, // Adjust for window margin
    ScreenHeight + 8 * 5 - 2, // Adjust for window margin and caption
    NULL, 
    NULL,
    inst,
    0
  );

  Init();

  ShowWindow(hwnd, TRUE);
  UpdateWindow(hwnd);

  
  MSG msg;
  memset(&msg, 0, sizeof(MSG));

  timer.Start();
  double delta = timer.DeltaTime();
  while(true)
  {
    if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      if(msg.message == WM_QUIT)
      {
        break;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    delta = timer.DeltaTime();
    UpdateSimulation(delta);
    Draw();
  }

  Cleanup();

  return 0;
}

void Init()
{
  GdiplusStartupInput startInput;
  GdiplusStartup(&gdiStartToken, &startInput, 0);

  HDC hdc = GetDC(hwnd);
  Graphics g(hdc);
  backBuffer = new Bitmap(ScreenWidth, ScreenHeight, &g);

  ScreenTransform =
    Matrix3x3::ScaleUniform(1.0f / MetersPerPixel) *
    Matrix3x3::Translation(0, -(ScreenHeight)) *
    Matrix3x3::Scale(1, -1); 


  Ball *b = new Ball();
  b->Mass = 10.0f;
  b->Position = Vector2D(4.2f, 6.2f);
  b->Velocity = Vector2D(0.0f, 0.0f);
  b->Radius = 0.55f;
  balls.push_back(b);
}

void Cleanup()
{
  delete backBuffer;
  GdiplusShutdown(gdiStartToken);

  for(Ball *b : balls) delete b;
  balls.clear();
}

void UpdateSimulation(double delta)
{
  for(Ball *b : balls)
  {
    Update(b, delta);
  }
}

void Draw()
{
  Graphics g(backBuffer);
  
  SolidBrush bg(Color(55, 55, 55));
  g.FillRectangle(&bg, 0, 0, ScreenWidth, ScreenHeight);


  SolidBrush br(Color(233, 255, 255));

  Vector2D p1(0.0f, 2.8f);
  Vector2D p2(8.0f, 2.4f);

  Pen p(Color(255, 0, 0));
  Point po1 = TransformToScreen(p1);
  Point po2 = TransformToScreen(p2);
  g.DrawLine(&p, po1, po2);
  
 
  for(Ball *b : balls)
  {
    Point closest = TransformToScreen(ClosestPointOnLine(b->Position, p1, p2));

    float dist = PointLineDistance(b->Position, p1, p2);
    if(dist < b->Radius)
    {      
      // Reflects the balls velocity on the line
      b->Velocity = Vector2D::Reflect(b->Velocity, (p2 - p1));

      // Make sure the ball is separated from the line.
      b->Position += (p2 - p1).Perpendicular().Unit() * -(b->Radius);
    }

    g.FillEllipse(&br, closest.X - 2, closest.Y - 2, 4, 4);

    Gdiplus::Point ballPos = TransformToScreen(b->Position);
    g.FillEllipse(&br,
      ballPos.X - MetersToPixels(b->Radius),
      ballPos.Y - MetersToPixels(b->Radius),
      MetersToPixels(2.0f * b->Radius),
      MetersToPixels(2.0f * b->Radius));    
  }


  HDC hdc = GetDC(hwnd);
  Graphics windowGraphics(hdc);
  windowGraphics.DrawImage(backBuffer, 0, 0, 0, 0, ScreenWidth, ScreenHeight, Unit::UnitPixel);

  ReleaseDC(hwnd, hdc);
}


LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch(msg)
  {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  default:
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}