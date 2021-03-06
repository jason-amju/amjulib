#include <AmjuFirst.h>
#include <DrawRect.h>
#include <AmjuGL.h>
#include <LoadVec2.h>
#include "GuiDialog.h"
#include <AmjuFinal.h>

namespace Amju
{
const char* GuiDialog::NAME = "gui-dialog";
const Colour DEFAULT_BG_COLOUR(0.5f, 0.5f, 0.5f, 0.5f);
const Colour DEFAULT_TITLE_COLOUR(0.1f, 0.1f, 0.1f, 0.5f);

GuiDialog::GuiDialog()
{
  m_hasTitleBar = true;
  m_drag = false;
  m_bgColour = DEFAULT_BG_COLOUR;
  m_titleColour = DEFAULT_TITLE_COLOUR;
}

bool GuiDialog::OnCursorEvent(const CursorEvent& e)
{
  if (m_drag)
  {
    Vec2f pos = GetLocalPos();
    pos += Vec2f(e.dx, e.dy);
    SetLocalPos(pos);
    return true; // ??
  }

  return GuiComposite::OnCursorEvent(e);
}

bool GuiDialog::OnMouseButtonEvent(const MouseButtonEvent& e)
{
  // Check if title bar clicked
  m_drag = false;
  if (HasTitleBar() && e.isDown)
  {
    if (GetRect(GetTitleBar()).IsPointIn(Vec2f(e.x, e.y)))
    {
      std::cout << "Title bar clicked!\n";
      m_drag = true;
      return true; // right?
    }
  }

  return GuiComposite::OnMouseButtonEvent(e);
}

void GuiDialog::SetTitle(const std::string& t)
{
  m_title.SetBgCol(Colour(0.25f, 0.25f, 0.25f, 1));
  m_title.SetDrawBg(true);
  m_title.SetFgCol(Colour(1, 1, 1, 1));
  m_title.SetFontSize(1.0f);
  m_title.SetLocalPos(GetLocalPos() + Vec2f(0, 0.1f));
  m_title.SetSize(Vec2f(GetSize().x, 0.1f));
  m_title.SetText(t);
  m_title.SetDrawBorder(true);

  m_hasTitleBar = true;
}

void GuiDialog::SetHasTitleBar(bool b)
{
  m_hasTitleBar = b;
}

bool GuiDialog::HasTitleBar() const
{
  return m_hasTitleBar;
}

GuiText* GuiDialog::GetTitleBar()
{
  return &m_title;
}

bool GuiDialog::Load(File* f)
{
  // Read in name, then filename of layout file. Load GUI widgets from this file.
  if (!f->GetDataLine(&m_name))
  {
    f->ReportError("Expected name for dialog");
    return false;
  }

// OK, this must have seemed like a good idea at the time but I can't
//  now remember WHY a dialog is split into 2 files. WHY??!!
#ifdef I_EVER_REMEMBER_WHY

  std::string filename;
  if (!f->GetDataLine(&filename))
  {
    f->ReportError("Expected layout file name for dialog");
    return false;
  }

  File f2;
  if (!f2.OpenRead(filename))
  {
    return false;
  }

#else

  // No filename for the second file, just continue on in the same file.
  File& f2 = *f;

#endif 

  // Like GuiWindow::Load, but we don't want to load in the name again. This would be
  //  only a generic name in the template file, not a unique name.
  //if (!GuiWindow::Load(&f2))
  //{
  //  return false;
  //}
  if (!LoadVec2(&f2, &m_localpos))
  {
    Assert(0);
    return false;
  }

  if (!LoadVec2(&f2, &m_size))
  {
    Assert(0);
    return false;
  }

  return LoadChildren(&f2);
}

void GuiDialog::Draw()
{
  // TODO Tiled background
  // Draw bg - TODO colour, texture ? Rounded corners etc ?
  Rect r = GetRect(this);
  AmjuGL::PushAttrib(AmjuGL::AMJU_TEXTURE_2D);
  AmjuGL::Disable(AmjuGL::AMJU_TEXTURE_2D);
  AmjuGL::SetColour(m_bgColour);
  DrawSolidRect(r);
  AmjuGL::PopAttrib(); 
  
  //GuiWindow::Draw();
  GuiComposite::Draw();

  if (m_hasTitleBar)
  {
    m_title.SetBgCol(m_titleColour);
    m_title.SetLocalPos(GetLocalPos() + Vec2f(0, 0.1f));
    m_title.Draw();
  }
}
}

