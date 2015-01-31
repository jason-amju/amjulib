#include "GuiCheck.h"

namespace Amju
{
const char* GuiCheck::NAME = "gui-check";

GuiCheck::GuiCheck() : m_checkedTex(nullptr), m_uncheckedTex(nullptr), m_changeValueFunc(nullptr), m_value(false)
{
}

void GuiCheck::ExecuteCommand() 
{
  GuiButton::ExecuteCommand();

  m_value = !m_value;
  if (m_changeValueFunc)
  {
    m_changeValueFunc(this, m_value);
  }
}

void GuiCheck::Set(Texture* checkedTex, Texture* uncheckedTex)
{
  m_checkedTex = checkedTex;
  m_uncheckedTex = uncheckedTex;
}

void GuiCheck::SetValue(bool b)
{
  m_value = b; 
  // TODO call callback?

  SetTexture(m_value ? m_checkedTex : m_uncheckedTex);
}

void GuiCheck::Draw()
{
  if (!IsVisible())
  {
    return;
  }

  PushColour();
  AmjuGL::SetColour(m_buttonColour);

  float a = 1.0f;
  if (!IsEnabled())
  {
    a = 0.5f;
  }
  MultColour(Colour(1, 1, 1, a));

  GuiImage::Draw();

  PopColour();
}

}


