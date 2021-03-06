#include "GuiSlider.h"

namespace Amju
{
const char* GuiSlider::NAME = "slider";

GuiSlider::GuiSlider()
{
  m_minX = -1;
  m_maxX = 1;
}
  
bool GuiSlider::Load(File* f) 
{
  if (!GuiButton::Load(f))
  {
    return false;
  }

  // TODO Load bg

  return true;
}

bool GuiSlider::IsEnabled() const
{
    return m_isEnabled;
}

void GuiSlider::Draw() 
{
  m_bg.SetParent(GetParent());
  m_bg.Draw();
  GuiButton::Draw();
}

bool GuiSlider::OnCursorEvent(const CursorEvent& ce)
{
  bool ret = GuiButton::OnCursorEvent(ce);

  // TODO move left/right
  if (m_isPressed)
  {
std::cout << "Moving slider! x: " << ce.x << ", dx: " << ce.dx << "\n";

    Vec2f pos = GetLocalPos();
    pos.x += ce.dx;
    if (pos.x > m_maxX)
    {
        pos.x = m_maxX;
    }
    if (pos.x < m_minX)
    {
        pos.x = m_minX;
    }
    SetLocalPos(pos);
  }

  return ret;
}

bool GuiSlider::OnMouseButtonEvent(const MouseButtonEvent& mbe) 
{
  bool ret = GuiButton::OnMouseButtonEvent(mbe);

  return ret;
}

}

