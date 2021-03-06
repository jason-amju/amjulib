#include <AmjuFirst.h>
#include "Rect.h"
#include <algorithm>
#include "AmjuAssert.h"
#include <AmjuFinal.h>

namespace Amju
{
Rect EmptyRect()
{
  const float F = 99999.9f;
  return Rect(F, -F, F, -F);
}

Rect::Rect() : m_xmin(0), m_xmax(0), m_ymin(0), m_ymax(0)
{
}

Rect::Rect(float xmin, float xmax, float ymin, float ymax) :
  m_xmin(xmin), m_xmax(xmax), m_ymin(ymin), m_ymax(ymax)
{
}

Rect::Rect(const Vec2f& corner1, const Vec2f& corner2)
{
  Set(
    std::min(corner1.x, corner2.x),
    std::max(corner1.x, corner2.x),
    std::min(corner1.y, corner2.y),
    std::max(corner1.y, corner2.y)
  );
}

void Rect::Set(float xmin, float xmax, float ymin, float ymax)
{
  m_xmin = xmin;
  m_xmax = xmax;
  m_ymin = ymin;
  m_ymax = ymax;
}

void Rect::SetIf(float x, float y)
{
  if (x < m_xmin) m_xmin = x;
  if (x > m_xmax) m_xmax = x;

  if (y < m_ymin) m_ymin = y;
  if (y > m_ymax) m_ymax = y;
}

void Rect::Union(const Rect& r)
{
  SetIf(r.m_xmax, r.m_ymax);
  SetIf(r.m_xmin, r.m_ymin);
}

void Rect::Translate(float x, float y)
{
  m_xmin += x;
  m_xmax += x;
  m_ymin += y;
  m_ymax += y;
}

void Rect::Scale(float x, float y)
{
  m_xmin *= x;
  m_xmax *= x;
  m_ymin *= y;
  m_ymax *= y;
}

Vec2f Rect::GetSize() const
{
  return Vec2f(m_xmax - m_xmin, m_ymax - m_ymin);
}

float Rect::GetMin(int axis) const
{
  Assert(axis < 2);
  const float* const f = &m_xmin;
  return f[axis * 2];
}

float Rect::GetMax(int axis) const
{
  Assert(axis < 2);
  const float* const f = &m_xmin;
  return f[axis * 2 + 1];
}

bool Rect::IsPointIn(const Vec2f& point) const
{
  return (
    point.x >= m_xmin && 
    point.x <= m_xmax && 
    point.y >= m_ymin && 
    point.y <= m_ymax);
}

/*
bool Rect::Intersects(const Rect& rect) const
{
}
*/

Rect Rect::CalcIntersectRegion(const Rect& r) const
{
  return Rect(
    std::max(m_xmin, r.m_xmin),
    std::min(m_xmax, r.m_xmax),
    std::max(m_ymin, r.m_ymin),
    std::min(m_ymax, r.m_ymax));
}
}
