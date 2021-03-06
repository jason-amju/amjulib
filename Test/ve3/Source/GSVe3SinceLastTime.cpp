#include <GuiFactory.h>
#include "GSVe3SinceLastTime.h"
#include "GuestbookWindow.h"
#include "MsgManager.h"

namespace Amju
{
static void OnBack(GuiElement*)
{
  TheGSVe3SinceLastTime::Instance()->GoBack();
}

GSVe3SinceLastTime::GSVe3SinceLastTime()
{
}

struct ForPlayerOrBroadcast
{
  ForPlayerOrBroadcast(int playerId) : m_playerId(playerId) {}

  bool operator()(const MsgManager::Msg& msg)
  {
    return msg.m_recipId == m_playerId || msg.m_recipId == MsgManager::BROADCAST_RECIP;
  }

  int m_playerId;
};

void GSVe3SinceLastTime::InitGB()
{
  GuestbookWindow* gw = (GuestbookWindow*)m_gui->GetElementByName("my-guestbook");
  Assert(gw);
  
  MsgManager::Msgs msgs = TheMsgManager::Instance()->GetMsgs(ForPlayerOrBroadcast(m_player->GetId()));
  gw->GetGBDisplay()->Init(msgs, true, false); // DO show reply buttons, right; not in 'sent' mode
}

void GSVe3SinceLastTime::OnDeactive()
{
  GSGui::OnDeactive();
}

void GSVe3SinceLastTime::OnActive()
{
  GSGui::OnActive(); // NOT direct base class, GSVe3Guestbook

  // Add GuestbookWindow type to GUI factory 
  //  - then gui txt file can use GuestbookWindow directly
  static bool addOnce = TheGuiFactory::Instance()->Add("ve3-guestbook", CreateGuestbookWindow);
  m_gui = LoadGui("gui-ve3-since-last-time.txt");
  Assert(m_gui);

  // TODO Set focus element, cancel element, command handlers
  GetElementByName(m_gui, "back-button")->SetCommand(OnBack);

  InitGB();
}

} // namespace
