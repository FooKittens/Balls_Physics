#include "Window.h"
#include "Ball.h"
#include "Line.h"
#include "Physics.h"

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
  this->globalRestitution = 1.0f;
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


  // Test Line
  lines.push_back(new Line(this, Vector2D(2.0f, 5.0f), Vector2D(6.0f, 5.0f)));
  lines.push_back(new Line(this, Vector2D(6.0f, 5.0f), Vector2D(6.0f, 1.2f)));
  lines.push_back(new Line(this, Vector2D(2.0f, 1.2f), Vector2D(6.0f, 1.2f)));
  lines.push_back(new Line(this, Vector2D(2.0f, 5.0f), Vector2D(2.0f, 1.2f)));

  lines.push_back(new Line(this, Vector2D(2.7f, 2.3f), Vector2D(5.2f, 3.8f)));
  
  
  ResetBalls();

  fpsStrBuffer = new WCHAR[20];
  memset(fpsStrBuffer, 0, sizeof(WCHAR) * 20);
  fpsFont = new Font(FontFamily::GenericMonospace(), 16);

  return true;
}

void Window::ResetBalls()
{
  for(Ball *pBall : balls)
  {
    delete pBall;
  }
  balls.clear();
  balls.shrink_to_fit();

  // Test ball
  Ball *b = new Ball(this);
  b->Initialize(0.5f, 0.25f, Vector2D(5.0f, 3.9f));
  balls.push_back(b);

  Ball *bb = new Ball(this);
  bb->Initialize(10.5f, 0.125f, Vector2D(4.5f, 4.7f));
  balls.push_back(bb);

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
        ball->ApplyImpulse(normalForce * -(1.0 + line->GetRestitution()) * mag);

        // Separate the ball from the line
        if(mag >= 0)
        {
          ball->Position += normalForce * -(ball->Radius - distance);
        }
        else
        {
          ball->Position += normalForce * (ball->Radius - distance);
        }
       
        /* Next we calculate the angular impulse by using the difference in
        velocities between the objects along the surface. */

        // We calculate the force parallell to the surface
        double d = Vector2D::Dot(ball->Velocity * ball->Mass, lineVec.Unit());
        // Calculate the actual distance between the ball's center
        double r = (closest - ball->Position).Length();
        // Calculate the angular impulse as the force parallell to the surface, scaled up by our scale factor for using metres
        double angImpulse = d * 256 / (3.141592 * ball->Mass) // 256 is scale factor for using metres
          -(1.0 + line->GetRestitution()) * r * ball->AngularVelocity;

        // Finally we apply the angular impulse!
        ball->ApplyAngularImpulse(angImpulse);
      }
    }
    // Update our ball
    Update(ball, deltaTime);
  }
}

void Window::UpdateGlobalRestitution()
{
  for(Line *line : lines)
  {
    line->SetRestitution(globalRestitution);
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


  SolidBrush fontBrush(Color(255, 255, 255));
  GetFpsString(fpsStrBuffer, 20);
  bufferGraphics->DrawString(
    fpsStrBuffer,
    lstrlenW(fpsStrBuffer),
    fpsFont,
    PointF(20, 20),
    NULL,
    &fontBrush
  );

  wchar_t buffer[20];
  swprintf(buffer, L"Restitution: %1.2f\0", globalRestitution);
  bufferGraphics->DrawString(
    buffer,
    lstrlenW(buffer),
    fpsFont,
    PointF(20, 35),
    NULL,
    &fontBrush
  );

  windowGraphics->DrawImage(backBuffer, 0, 0, 0, 0, width, height, Unit::UnitPixel);
}


LRESULT CALLBACK Window::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  int keycode;

  switch(msg)
  {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_KEYDOWN:
    keycode = LOWORD(wParam);
    switch(keycode)
    {
    case VK_UP:
      globalRestitution += 0.05;
      UpdateGlobalRestitution();
      break;
    case VK_DOWN:
      globalRestitution -= 0.05;
      UpdateGlobalRestitution();
      break;
    case VK_SPACE:
      ResetBalls();
      break;
    }
    return 0;
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
}

LRESULT CALLBACK Window::StaticWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if(msg == WM_CREATE)
  {
    Window *pWindow = (Window *)((LPCREATESTRUCT)lParam)->lpCreateParams;
    if(pWindow)
    {
      SetWindowLong(hwnd, GWL_USERDATA, (long)pWindow);
    }
  }

  Window *pWindow = (Window *)GetWindowLong(hwnd, GWL_USERDATA);
  if(pWindow)
  {
    return pWindow->WinProc(hwnd, msg, wParam, lParam);
  }
  
  return DefWindowProc(hwnd, msg, wParam, lParam);
}