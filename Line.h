#ifndef LINE_H
#define LINE_H

#include <gdiplus.h>
#include "Vector2D.h"
#include "Window.h"

using namespace Gdiplus;

extern Gdiplus::Point TransformToWindow(const Vector2D&);

class Line
{
public:
  // Constructor
  Line(Window *window, Color &color = Color(255, 255, 255));
  Line(Window *window, const Vector2D &from, const Vector2D &to, Color &color = Color(255, 255, 255));
  Line(Window *window, const Vector2D &from, const Vector2D &to,
    double restitution, double frictionCoeff,
    Color &color = Color(255, 255, 255));

  // Destructor
  ~Line();

  void Draw(Graphics *g) const;

  // Accessors
  const Vector2D& GetStart() const;
  const Vector2D& GetEnd() const;
  const float GetFrictionCoeff() const;
  const float GetRestitution() const;
  const Pen& GetPen() const;

  void SetStart(const Vector2D &start);
  void SetEnd(const Vector2D &end);
  void SetFrictionCoeff(float f);
  void SetRestitution(float r);
  void SetColor(const Color &color);

private:
  Vector2D start, end;
  double frictionCoeff, restitution;
  Pen pen;
  Window *window;
};

Line::Line(Window *window, Color &color)
  :pen(color)
{
  this->window = window;
  restitution = 1.0;
  frictionCoeff = 1.0;
}

Line::Line(Window *window, const Vector2D &from, const Vector2D &to, Color &color)
  :pen(color)
{
  this->window = window;
  start = from;
  end = to;
  frictionCoeff = 1.0;
  restitution = 1.0;
}

Line::Line(Window *window, const Vector2D &from, const Vector2D &to, double res, double fri, Color &color)
  :pen(color)
{
  this->window = window;
  start = from;
  end = to;
  restitution = res;
  frictionCoeff = fri;
}

void Line::Draw(Graphics *g) const
{
  g->DrawLine(&pen,
    window->TransformToWindow(start),
    window->TransformToWindow(end));
}

// Inlined accessors
inline const Vector2D& Line::GetStart() const { return start; }
inline const Vector2D& Line::GetEnd() const { return end; }
inline const float Line::GetFrictionCoeff() const { return frictionCoeff; }
inline const float Line::GetRestitution() const { return restitution; }
inline const Pen& Line::GetPen() const { return pen; }

inline void Line::SetStart(const Vector2D &start) { this->start = start; }
inline void Line::SetEnd(const Vector2D &end) { this->end = end; }
inline void Line::SetFrictionCoeff(float f) { frictionCoeff = f; }
inline void Line::SetRestitution(float r) { restitution = r; }
inline void Line::SetColor(const Color &color) { pen.SetColor(color); }

#endif