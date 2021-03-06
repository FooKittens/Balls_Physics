#ifndef MATRIX3X3_H
#define MATRIX3X3_H
#include <cmath>

// We need to use 3x3 matrices to support translation matrices.
struct Matrix3x3
{
  Matrix3x3() { 
    m11 = 0; m12 = 0; m13 = 0;
    m21 = 0; m22 = 0; m23 = 0;
    m31 = 0; m32 = 0; m33 = 0;
  }
  Matrix3x3(double m11, double m12, double m13,
            double m21, double m22, double m23,
            double m31, double m32, double m33)
  { 
    this->m11 = m11; this->m12 = m12; this->m13 = m13;
    this->m21 = m21; this->m22 = m22; this->m23 = m23;
    this->m31 = m31; this->m32 = m32; this->m33 = m33;
  }
  
  static Matrix3x3 Identity();
  static Matrix3x3 Rotation(double rad);
  static Matrix3x3 Translation(double x, double y);
  static Matrix3x3 ScaleUniform(double scale);
  static Matrix3x3 Scale(double sx, double sy);

  // m[row][column]
  double m11; double m12; double m13;
  double m21; double m22; double m23;
  double m31; double m32; double m33;

};

inline Matrix3x3 operator*(const Matrix3x3 &lhs, const Matrix3x3 &rhs)
{
  Matrix3x3 result;
  // Row 1
  result.m11 = lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21 + lhs.m13 * rhs.m31;
  result.m12 = lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22 + lhs.m13 * rhs.m32;
  result.m13 = lhs.m11 * rhs.m31 + lhs.m12 * rhs.m32 + lhs.m13 * rhs.m33;

  // Row 2
  result.m21 = lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21 + lhs.m23 * rhs.m31;
  result.m22 = lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22 + lhs.m23 * rhs.m32;
  result.m23 = lhs.m21 * rhs.m13 + lhs.m22 * rhs.m23 + lhs.m23 * rhs.m33;

  // Row 3
  result.m31 = lhs.m31 * rhs.m11 + lhs.m32 * rhs.m21 + lhs.m33 * rhs.m31;
  result.m32 = lhs.m31 * rhs.m12 + lhs.m32 * rhs.m22 + lhs.m33 * rhs.m32;
  result.m33 = lhs.m31 * rhs.m13 + lhs.m32 * rhs.m23 + lhs.m33 * rhs.m33;
  
  return result;
}

inline Matrix3x3 Matrix3x3::Identity()
{
  return Matrix3x3(1, 0, 0,
                   0, 1, 0,
                   0, 0, 1);
}

inline Matrix3x3 Matrix3x3::Rotation(double rad)
{
  return Matrix3x3(cos(rad), sin(rad), 0,
                   -sin(rad), cos(rad), 0,
                   0, 0, 0);
}

inline Matrix3x3 Matrix3x3::Translation(double x, double y)
{
  return Matrix3x3(1, 0, 0,
                   0, 1, 0,
                   x, y, 1);
}

inline Matrix3x3 Matrix3x3::ScaleUniform(double scale)
{
  return Matrix3x3(scale, 0, 0,
                   0, scale, 0,
                   0, 0, 1);
}

inline Matrix3x3 Matrix3x3::Scale(double sx, double sy)
{
  return Matrix3x3(sx, 0, 0,
                   0, sy, 0,
                   0, 0, 1);
}

#endif