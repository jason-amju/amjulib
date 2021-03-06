// Amjulib - cross platform game engine
// (c) Copyright Jason Colman 2000-2018

#include <AmjuHash.h>
#include <Lerp.h>
#include <LoadVec2.h>
#include <GuiSprite.h>
#include <StringUtils.h>

namespace Amju
{
const char* GuiSprite::NAME = "gui-sprite";

Texture* GuiSprite::GetTexture()
{
  return GuiImage::GetTexture();
}

void GuiSprite::Draw()
{
  // Add to batch of stuff to draw
  AddToBatch();

  // Store current state of transform and colour, which we use later when
  //  we draw the batch of all tris using the current texture.
  // TODO At some point, we will need to store other stuff, e.g. current shader.
  const Vec2f& pos = GetCombinedPos();
  const Vec2f& size = GetSize();
  AmjuGL::PushMatrix();
  AmjuGL::Translate(pos.x, pos.y, 0);
  AmjuGL::Scale(size.x, size.y, 1);
  m_combinedTransform.ModelView();
  m_combinedColour = AmjuGL::GetColour();
  AmjuGL::PopMatrix();
}

void GuiSprite::Animate(float animValue)
{
  // Going forward, add 1 because max cell is part of the anim sequence.
  float d = 0.99f;
  if (m_maxCell < m_minCell)
  {
    // TODO
    // TODO If maxCell < minCell, we should do d = -1 ?
    // Doesn't quite work.
  }
  int cell = static_cast<int>(Lerp(
    static_cast<float>(m_minCell), 
    static_cast<float>(m_maxCell) + d, 
    animValue));

  SetCell(cell);
}

void GuiSprite::Normalise(int& cell) const
{
  const int maxCells = m_numCellsXY.x * m_numCellsXY.y;
  if (cell < 0)
  {
    cell += maxCells;
  }
  Assert(cell >= 0);
  if (cell >= maxCells)
  {
    cell -= maxCells;
  }
  Assert(cell < maxCells);
}

void GuiSprite::SetCellRange(int minCell, int maxCell)
{
  Normalise(minCell);
  Normalise(maxCell);

  m_minCell = minCell;
  m_cell = minCell;
  m_maxCell = maxCell;
}

void GuiSprite::SetCell(int cell)
{
  Normalise(cell);

  m_cell = cell;
}

void GuiSprite::AddToTrilist(AmjuGL::Tris& tris)
{
  if (!IsVisible())
  {
    return;
  }

  float du = 1.f / static_cast<float>(m_numCellsXY.x);
  float dv = 1.f / static_cast<float>(m_numCellsXY.y);
  float u0 = du * static_cast<float>(m_cell % m_numCellsXY.x);
  float v0 = 1.f - dv * static_cast<float>(m_cell / m_numCellsXY.x); // x, not y
  float u1 = u0 + du;
  float v1 = v0 - dv;

  const float Z = 0;
  // Corners: a unit square, transformed by whatever the current state of
  //  the modelview matrix was when Draw was called.
  Vec3f v[4] = 
  {
    Vec3f(1, -1, Z) * m_combinedTransform,
    Vec3f(1,  0, Z) * m_combinedTransform,
    Vec3f(0,  0, Z) * m_combinedTransform,
    Vec3f(0, -1, Z) * m_combinedTransform,
  };

  AmjuGL::Vert verts[4] =
  {
    AmjuGL::Vert(v[0].x, v[0].y, v[0].z,   u1, v1,   0, 1, 0), // x, y, z, u, v, nx, ny, nz  
    AmjuGL::Vert(v[1].x, v[1].y, v[1].z,   u1, v0,   0, 1, 0),
    AmjuGL::Vert(v[2].x, v[2].y, v[2].z,   u0, v0,   0, 1, 0),
    AmjuGL::Vert(v[3].x, v[3].y, v[3].z,   u0, v1,   0, 1, 0)
  };

  AmjuGL::Tri tri;
  tri.Set(verts[0], verts[1], verts[2]);
  // Set the vertex colours to the colour which was current when Draw was called
  tri.SetColour(m_combinedColour.m_r, m_combinedColour.m_g, m_combinedColour.m_b, m_combinedColour.m_a);
  tris.push_back(tri);

  tri.Set(verts[0], verts[2], verts[3]);
  tri.SetColour(m_combinedColour.m_r, m_combinedColour.m_g, m_combinedColour.m_b, m_combinedColour.m_a);
  tris.push_back(tri);
}

bool GuiSprite::Load(File* f)
{
  if (!GuiImage::Load(f))
  {
    return false;
  }

  // TODO TEMP TEST - does this make any difference when image is mipmapped?
  m_texture->SetWrapMode(AmjuGL::AMJU_TEXTURE_CLAMP);

  // Hash the texture resource name, so we can group all sprites using the same
  //  texture.
  m_texHash = HashString(m_texture->GetResName());

  // Get sprite sheet layout info
  if (!LoadVec2(f, &m_numCellsXY))
  {
    f->ReportError("Expected sprite sheet number of cells in x and y.");
    return false;
  }

  // Get current cell. We can specify 2 cell numbers here, in which case we 
  //  animate over the cell range.

  std::string s;
  if (!f->GetDataLine(&s))
  {
    f->ReportError("Expected cell number (1 or 2) for sprite.");
    return false;
  }

  Strings strs = Split(s, ',');
  Assert(!strs.empty());
  m_cell = ToInt(strs[0]);
  m_minCell = m_cell;
  m_maxCell = m_cell;
  if (strs.size() == 2)
  {
    m_maxCell = ToInt(strs[1]);
  }

  return true;
}

}
