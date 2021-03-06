#include <AmjuFirst.h>
#include "Tri.h"
#include "AmjuGL.h"
#include <AmjuFinal.h>

namespace Amju
{
void Tri::Transform(const Matrix& m)
{
  m_verts[0] = m_verts[0] * m; // TODO no *= ??
  m_verts[1] = m_verts[1] * m; 
  m_verts[2] = m_verts[2] * m; 
}

void Tri::Translate(const Vec3f& v)
{
  m_verts[0] += v;
  m_verts[1] += v;
  m_verts[2] += v;
}

Vec3f Tri::CalcNormal() const
{
  Vec3f v1 = m_verts[1] - m_verts[0];
  Vec3f v2 = m_verts[2] - m_verts[0];
  Vec3f n = CrossProduct(v1, v2);
  n.Normalise();
  return n;
}

void Tri::Draw()
{
  AmjuGL::Vec3 v0(m_verts[0].x, m_verts[0].y, m_verts[0].z);
  AmjuGL::Vec3 v1(m_verts[1].x, m_verts[1].y, m_verts[1].z);
  AmjuGL::Vec3 v2(m_verts[2].x, m_verts[2].y, m_verts[2].z);

  AmjuGL::DrawLine(v0, v1);
  AmjuGL::DrawLine(v1, v2);
  AmjuGL::DrawLine(v2, v0);
}
}
