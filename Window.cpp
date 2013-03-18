#include "Window.h"
#include "Ball.h"
#include "Line.h"
#include "Physics.h"
#include <cmath>

extern const double MetersPerPixel;

using Gdiplus::Graphics;

Window::Window(HINSTANCE instance, UINT width, UINT height)
{
  this->width = width;
  this->height = height;
  this->appInstance = instance;
  this->gdiStartToken = 0;
  this->bufferGraphics = nullptr;
  this->windowGraphics = nullptr;
  this->backBuffer = nullptr;
  globalRestitution = 1.0;
}

Window::~Window()
{
  delete backBuffer;
  delete bufferGraphics;
  delete windowGraphics;
  delete fpsFont;
  delete fpsStrBuffer;

  // Remove all resources used by our balls.
  for(Ball *b : balls) delete b;

  // Clear the pointers to the balls.
  balls.clear();

  // Must be called last, can't remove gdi+ objects once its closed.
  GdiplusShutdown(gdiStartToken);
}

bool Window::Initialize()
{
 const TCHAR *appClass = TEXT("BallClass");

  WNDCLASS wclass;
  wclass.cbClsExtra = 0;
  wclass.cbWndExtra = 0;
  wclass.hbrBackground = 0;
  wclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wclass.hInstance = appInstance;
  wclass.lpfnWndProc = &StaticWinProc;
  wclass.lpszClassName = appClass;
  wclass.lpszMenuName = NULL;
  wclass.style = 0;


  if(!RegisterClass(&wclass))
  {
    return false;
  }

  hWindow = CreateWindow(
    appClass,
    "Physics Simulation",
    WS_OVERLAPPED | WS_SYSMENU,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    width + 8 * 2, // Adjust for window margin
    height + 8 * 5 - 2, // Adjust for window margin and caption
    NULL, 
    NULL,
    appInstance,
    this
  );

  if(!hWindow)
  {
    return false;
  }

  GdiplusStartupInput startInput;
  GdiplusStartup(&gdiStartToken, &startInput, 0);

  HDC hdc = GetDC(hWindow);
  windowGraphics = new Graphics(hdc);
  backBuffer = new Bitmap(width, height, windowGraphics);
  bufferGraphics = new Graphics(backBuffer);

  // Transform used to turn our coordinates in meters into pixels.
  // Also inverts the y axis to produce a more typical coordinate system.
  screenTransformMat =
    Matrix3x3::ScaleUniform(1.0f / MetersPerPixel) *
    Matrix3x3::Translation(0, -((int)height)) *
    Matrix3x3::Scale(1, -1); 

  // Test ball
  Ball *b = new Ball(this);
  b->Initialize(6.5f, 0.125f, Vector2D(5.0f, 3.9f));
  //balls.push_back(b);

  Ball *bb = new Ball(this);
  bb->Initialize(2.5f, 0.25f, Vector2D(4.5f, 4.7f));
  balls.push_back(bb);

  // Test Line
  lines.push_back(new Line(this, Vector2D(2.0f, 5.0f), Vector2D(6.0f, 5.0f)));
  lines.push_back(new Line(this, Vector2D(6.0f, 5.0f), Vector2D(6.0f, 1.2f)));
  lines.push_back(new Line(this, Vector2D(2.0f, 1.2f), Vector2D(6.0f, 1.2f)));
  lines.push_back(new Line(this, Vector2D(2.0f, 5.0f), Vector2D(2.0f, 1.2f)));

  lines.push_back(new Line(this, Vector2D(3.1f, 2.3f), Vector2D(5.2f, 3.8f)));
  
  for(Line *line : lines)
  {
    line->SetRestitution(1.0f);
  }


  fpsStrBuffer = new WCHAR[20];
  memset(fpsStrBuffer, 0, sizeof(WCHAR) * 20);
  fpsFont = new Font(FontFamily::GenericMonospace(), 16);

  return true;
}



bool Window::Run()
{
  ShowWindow(hWindow, TRUE);
  UpdateWindow(hWindow);

  MSG msg;
  memset(&msg, 0, sizeof(MSG));

  timer.Start();
  double delta = timer.DeltaTime();

  frameTimer = 0.0;
  frames = 0;
  lastFps = 0;

  // Maximum simulation time used as a temporary fix for physics breaking when moving window.
  double maxStep = 0.01;
  
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

    // Temporary.
    if(delta > maxStep)
    {
      UpdateSimulation(maxStep);
    }
    else
    {
      UpdateSimulation(delta);
    }
    

    Draw();
    frames++;
    frameTimer += delta;
    if(frameTimer >= 1.0)
    {
      lastFps = frames;
      frameTimer = 0;
      frames = 0;
    }
  }

  return true;
}

void Window::UpdateSimulation(double deltaTime)
{
  for(Ball *ball : balls)
  {

    for(Line *line : lines)
    {
      Vector2D closest = ClosestPointOnLine(ball->Position, (*line));

      if(closest.X < line->GetStart().X)
      {
        closest = line->GetStart();
      }
      else if(closest.X > line->GetEnd().X)
      {
        closest = line->GetEnd();
      }

      float distance = (ball->Position - closest).Length();

      /* If the distance between the balls center and the line
       * is smaller than the balls radius, we have a collision. */
      if(distance < ball->Radius)
      {
        Vector2D lineVec = line->GetEnd() - line->GetStart();
        
        /* Newtons laws of physics gives us that for every action
         * theres an equal and opposite reaction. 
         * Because of this we can calculate the force applied to the ball
         * by calculating the force the ball exerts on the line. */

        // The lines normal as a unit vector will be the direction.
        Vector2D normalForce = lineVec.Perpendicular().Unit();

        /* In order to get the magnitude of the force we will calculate
         * the impulse caused by the ball.
         * Since the ball must not penetrate the line we can assume
         * that the force must be equal to whatever force the ball
         * exerts on the line along its normal. */

        // Here we project the balls momentum on the lines normal.
        double mag = Vector2D::Dot(ball->Velocity * ball->Mass, normalForce);

        /* The response will now be to add the velocity change caused by
         * the opposite impulse. */
        //ball->Velocity += normalForce * ((-(0.85 + line->GetRestitution()) * mag) / ball->Mass);
        //ball->ApplyImpulse(normalForce * -(1.0 + line->GetRestitution()) * mag);

        Vector2D rel = (ball->Position - closest);

        double inertia = 3.141592 * pow(ball->Radius, 2.0) / 4;

        double angCoeff = pow(Vector2D::Dot(rel.Perpendicular(), normalForce), 2.0) / inertia;

        Vector2D totVel = ball->Velocity + rel.Perpendicular() * ball->AngularVelocity;

        double j = (-(1.0 + line->GetRestitution()) * 
          Vector2D::Dot(totVel, normalForce)) /
          ((1 / ball->Mass) + angCoeff);

        ball->Velocity += normalForce * (j / ball->Mass);
        ball->AngularVelocity += Vector2D::Dot(rel.Perpendicular(), normalForce * j) / inertia;
            
        // Separate the ball from the line
        if(mag >= 0)
        {
          ball->Position += normalForce * -(ball->Radius - distance);
        }
        else
        {
          ball->Position += normalForce * (ball->Radius - distance);
        }
       

        //double d = Vector2D::Dot(ball->Velocity * ball->Mass, lineVec.Unit());
        //
        //double angImpulse = d
        //  + (1.0 + line->GetRestitution()) * r * ball->AngularVelocity;
        //ball->ApplyAngularImpulse(angImpulse);

        //double coeff = Vector2D::Dot(pointVel, normalForce);
        

      }
    }
    // Update our ball
    Update(ball, deltaTime);
  }
}

Gdiplus::Point Window::TransformToWindow(const Vector2D &vec) const
{
  Vector2D transformed = Vector2D::Transform(vec, screenTransformMat);

  return Gdiplus::Point(static_cast<int>(transformed.X),
    static_cast<int>(transformed.Y));
}

void Window::GetFpsString(WCHAR *buffer, int size)
{
  if(size < 11) return;

  double fps = frames + (lastFps * (1.0 - frameTimer));
  swprintf(buffer, L"FPS: %4.2f ", fps);
}

void Window::Draw()
{  
  Gdiplus::SolidBrush clearBrush(Color(0, 0, 0));

  bufferGraphics->FillRectangle(&clearBrush, 0, 0, width, height);


  for(Line *line : lines)
  {
    line->Draw(bufferGraphics);
  }

  for(Ball *ball : balls)
  {
    ball->Draw(bufferGraphics);
  }

  GetFpsString(fpsStrBuffer, 20);
  bufferGraphics->DrawString(
    fpsStrBuffer,
    lstrlenW(fpsStrBuffer),
    fpsFont,
    PointF(20, 20),
    NULL,
    &SolidBrush(Color(255, 255, 255))
  ); 

  wchar_t resBuffer[20];
  swprintf(resBuffer, L"Restitution: %1.2f\0", globalRestitution);

  bufferGraphics->DrawString(
    resBuffer,
    lstrlenW(resBuffer),
    fpsFont,
    PointF(20, 38),
    NULL,
    &SolidBrush(Color(255, 255, 255))
  );

  windowGraphics->DrawImage(backBuffer, 0, 0, 0, 0, width, height, Unit::UnitPixel);
}

LRESULT CALLBACK Window::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static int key;

  switch(msg)
  {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_KEYDOWN:
    key = LOWORD(wParam);
    switch(key)
    {
    case VK_UP:
      globalRestitution += 0.05;
      globalRestitution = min(globalRestitution, 1.0);
      UpdateGlobalRestitution();
      break;
    case VK_DOWN:
      globalRestitution -= 0.05;
      globalRestitution = max(globalRestitution, 0.0);
      UpdateGlobalRestitution();
      break;
    }
    return 0;
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }

}

void Window::UpdateGlobalRestitution()
{
  for(Line *line : lines)
  {
    if(line)
    {
      line->SetRestitution(globalRestitution);
    }
  }
}

LRESULT CALLBACK Window::StaticWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static Window *pWinInstance = nullptr;

  if(msg == WM_CREATE)
  {
    pWinInstance = (Window *) ((LPCREATESTRUCT) lParam)->lpCreateParams;
    SetWindowLong(hwnd, GWL_USERDATA, (long)pWinInstance);
  }

  pWinInstance = (Window *)GetWindowLong(hwnd, GWL_USERDATA);

  if(pWinInstance)
  {
    return pWinInstance->WinProc(hwnd, msg, wParam, lParam);
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}