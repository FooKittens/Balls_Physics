#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <cmath>
#include "Matrix3x3.h"


// Struct for a vector in the plane.
struct Vector2D
{
  Vector2D() { X = 0; Y = 0; };
  Vector2D(double x, double y) { X = x; Y = y; }

  // copy constructor
  Vector2D(const Vector2D& vec) { X = vec.X; Y = vec.Y; }

  double X;
  double Y;

  // Retrieves the perpendicular vector to this vector.
  const Vector2D Perpendicular() const;

  static double Dot(const Vector2D &lhs, const Vector2D &rhs);
  static double Cross(const Vector2D &lhs, const Vector2D &rhs);
  static Vector2D Transform(const Vector2D &vec, const Matrix3x3 &matrix);
  static Vector2D Reflect(const Vector2D &vec, const Vector2D &line);

  // returns a unit version of this vector
  const Vector2D Unit() const;

  // Normalizes this vector
  void Normalize();

  double Length() const;
  double LengthSquared() const;


  // Operators
  void operator+=(const Vector2D &rhs);

};




// Inlines

// Multiplication with scalar
inline const Vector2D operator*(const Vector2D &vec, double scalar)
{
  return Vector2D(vec.X * scalar, vec.Y * scalar);
}

// Vector addition
inline const Vector2D operator+(const Vector2D &lhs, const Vector2D &rhs)
{
  return Vector2D(lhs.X + rhs.X, lhs.Y + rhs.Y);
}

inline const Vector2D operator-(const Vector2D &lhs, const Vector2D &rhs)
{
  return Vector2D(lhs.X - rhs.X, lhs.Y - rhs.Y);
}

inline const Vector2D Vector2D::Perpendicular() const
{
  return Vector2D(this->Y, -this->X);
}

inline double Vector2D::Dot(const Vector2D &lhs, const Vector2D &rhs)
{
  return (lhs.X * rhs.X + lhs.Y * rhs.Y);
}

inline double Vector2D::Cross(const Vector2D &lhs, const Vector2D &rhs)
{
  return (lhs.X * rhs.Y) - (lhs.Y * rhs.X);
}

inline const Vector2D Vector2D::Unit() const
{
  double len = Length();
  return Vector2D(X / len, Y / len);
}

inline double Vector2D::LengthSquared() const
{
  return this->X * this->X + this->Y * this->Y;
}

inline double Vector2D::Length() const
{
  return sqrt(this->LengthSquared());
}

inline Vector2D Vector2D::Transform(const Vector2D &vec, const Matrix3x3 &matrix)
{
  // [ax ay 1] * [b11 b12 b13 : b21 b22 b23 : b31 b32 b33]
  // Note that we're not interested in storing the would-be Z value.
  double x = vec.X * matrix.m11 + vec.Y * matrix.m21 + 1 * matrix.m31;
  double y = vec.X * matrix.m12 + vec.Y * matrix.m22 + 1 * matrix.m32;

  return Vector2D(
    vec.X * matrix.m11 + vec.Y * matrix.m21 + 1 * matrix.m31,
    vec.X * matrix.m12 + vec.Y * matrix.m22 + 1 * matrix.m32);
}

inline Vector2D Vector2D::Reflect(const Vector2D &vec, const Vector2D &line)
{
  return line * (2.0 * Vector2D::Dot(vec, line) / Vector2D::Dot(line, line)) - vec;
}

inline void Vector2D::operator+=(const Vector2D &rhs)
{
  this->X += rhs.X;
  this->Y += rhs.Y;
}

#endif