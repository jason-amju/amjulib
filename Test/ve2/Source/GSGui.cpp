#include <AmjuFirst.h>
#include "GSGui.h"
#include <AmjuGL.h>

#define GLEW_STATIC
#include <OpenGL.h> // naughty, using OpenGL directly here to scroll texture

#include <CursorManager.h>
#include <Timer.h>
#include <EventPoller.h>
#include <GuiText.h>
#include "LurkMsg.h"
#include "Ve1SceneGraph.h"
#include "ROConfig.h"
#include "LocalPlayer.h"
#include "HeartCount.h"
#include "GameConsts.h"
#include <AmjuFinal.h>

namespace Amju
{
GSGui::GSGui()
{
  m_showLurk = false;
}

void GSGui::Update()
{
  GSBase::Update();
  
  if (m_gui)
  {
    m_gui->SetVisible(true);
  }

  if (m_showLurk)
  {
    static Lurker* lurker = TheLurker::Instance();
    lurker->Update();
    if (m_gui && lurker->IsDisplayingMsg())
    {
      m_gui->SetVisible(false);
    }
  }
}

void GSGui::Draw()
{
  AmjuGL::PushAttrib(AmjuGL::AMJU_LIGHTING | AmjuGL::AMJU_DEPTH_WRITE | AmjuGL::AMJU_DEPTH_READ);
  AmjuGL::Disable(AmjuGL::AMJU_LIGHTING);
  AmjuGL::Disable(AmjuGL::AMJU_DEPTH_WRITE);
  AmjuGL::Disable(AmjuGL::AMJU_DEPTH_READ);

#ifndef AMJU_USE_ES2
  // TODO This won't work on DX or Wii either -- TODO Fix properly
  glMatrixMode(GL_TEXTURE); // naughty!
  glPushMatrix();
  static float f = 0;

  AmjuGL::Translate(-f, 0, 0);

  static float SPEED = 0.05f; // Too soon for ROConfig()->GetFloat("scope-speed");
  f += SPEED * TheTimer::Instance()->GetDt();
#endif
  m_bgImage.Draw();
  
#ifndef AMJU_USE_ES2
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
#endif
  
  AmjuGL::PopAttrib();

  GSBase::Draw();
}

void GSGui::Draw2d()
{
  GSBase::Draw2d();

  if (m_gui)
  {
    m_gui->Draw();
  }

  if (m_showLurk)
  {
    TheLurker::Instance()->Draw();
  }

  TheCursorManager::Instance()->Draw();
}

void GSGui::OnActive()
{
  GSBase::OnActive();
  AmjuGL::SetClearColour(Colour(0, 0, 0, 1));

  ClearGuiSceneGraph();

  if (!m_bgImage.OpenAndLoad("bgimage.txt"))
  {
std::cout << "Failed to load GUI bg image!\n";
    Assert(0);
  }
  m_bgImage.GetTexture()->SetWrapMode(AmjuGL::AMJU_TEXTURE_WRAP);
}

void GSGui::OnDeactive()
{
  GSBase::OnDeactive();

  // Reload GUI every time we activate. 
  // (This makes it reasonable to add gui elements as listeners when they are created)
  if (TheEventPoller::Instance()->HasListener(m_gui))
  {
    TheEventPoller::Instance()->RemoveListener(m_gui);
  }
  m_gui = 0;
}

bool GSGui::LoadCogTestBg()
{
  if (!m_bgImage.OpenAndLoad("cogbgimage.txt"))
  {
std::cout << "Failed to load cog test bg image!\n";
    Assert(0);
    return false;
  }
  m_bgImage.GetTexture()->SetWrapMode(AmjuGL::AMJU_TEXTURE_WRAP);

  return true;
}

void GSGui::DrawCogTestBg()
{
  AmjuGL::PushAttrib(AmjuGL::AMJU_LIGHTING | AmjuGL::AMJU_DEPTH_WRITE | AmjuGL::AMJU_DEPTH_READ);
  AmjuGL::Disable(AmjuGL::AMJU_LIGHTING);
  AmjuGL::Disable(AmjuGL::AMJU_DEPTH_WRITE);
  AmjuGL::Disable(AmjuGL::AMJU_DEPTH_READ);

#ifndef AMJU_USE_ES2
  glMatrixMode(GL_TEXTURE); // naughty!
  glPushMatrix();
  static float f = 0;
  //AmjuGL::RotateZ(f);

  // Scrolling might be distracting
  AmjuGL::Translate(-f, 0, 0);

  static float SPEED = 0.001f; // Too soon for ROConfig()->GetFloat("scope-speed");
  f += SPEED * TheTimer::Instance()->GetDt();
#endif
  
  m_bgImage.Draw();
  
#ifndef AMJU_USE_ES2
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
#endif
  
  AmjuGL::PopAttrib();

  GSBase::Draw();
}

void GSGui::UpdateHeartCount()
{
  if (m_gui)
  {
    GuiText* text = (GuiText*)GetElementByName(m_gui, "score-num");
    if (text)
    {
      int h = 0;
      if (GetPlayerCount(SCORE_KEY, &h))
      {
        text->SetText(ToString(h));
      }
      else
      {
        text->SetText("");
      }
    }
  }
}
} // namespace
