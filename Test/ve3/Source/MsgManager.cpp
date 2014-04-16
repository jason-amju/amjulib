#include <AmjuFirst.h>
#include <iostream>
#include <StringUtils.h>
#include <SafeUtils.h>
#include <Timer.h>
#include <UrlUtils.h>
#include <Game.h>
#include <StringUtils.h>
#include <TimePeriod.h>
#include <XmlParser2.h>
#include "MsgManager.h"
#include "ReqSendMsg.h"
#include "ReqGetNewMsgs.h"
#include "GSMain.h"
#include "Player.h"
#include "ReqMsgRead.h"
#include "ChatConsole.h"
#include "BroadcastConsole.h"
#include "Ve1OnlineReqManager.h"
#include "TextToSpeech.h"
#include "ROConfig.h"
#include "HeartCount.h"
#include "Achievement.h"
#include "LurkMsg.h"
#include "GameConsts.h"
#include <AmjuFinal.h>

#define SEND_DEBUG

namespace Amju
{
void AddTradeInfo(std::string* msg, int giveNum, int recvNum, TradeType tt)
{
  std::string info = "\n<trade><give>" + ToString(giveNum) + "</give><recv>" +
    ToString(recvNum) + "</recv><type>" + ToString((int)tt) + "</type></trade>";

  *msg = *msg + info;
}

bool MsgManager::Msg::IsTrade() const
{
  bool b = StringContains(m_text, "<trade>");
  return b;
}
    
std::string MsgManager::Msg::GetStrippedText() const
{
  // HACK: Strip final line, if it begins '<' and ends '>'
  std::string res = m_text;
  unsigned int i = res.find_last_of('\n');
  if (i != std::string::npos)
  {
    std::string lastLine = res.substr(i);
    Trim(&lastLine);
    // Look for <> -- strip this last line from res?
    if (!lastLine.empty() && lastLine.front() == '<' && lastLine.back() == '>')
    {
      res = res.substr(0, i);
    }
  }
  return res; 
}

void MsgManager::Msg::GetTradeInfo(int* giveNum, int* recvNum, TradeType* tt) const
{
  Assert(IsTrade());

  PXml xml = ParseXml(m_text.c_str());
  PXml p = xml.getChildNode(0);
  if (SafeStrCmp(p.getName(), "trade"))
  {
    PXml xmlGive = p.getChildNode(0);
    if (!SafeStrCmp(xmlGive.getName(), "give"))
    {
      Assert(0);
      return;
    }
    PXml xmlRecv = p.getChildNode(1);
    if (!SafeStrCmp(xmlRecv.getName(), "recv"))
    {
      Assert(0);
      return;
    }
    PXml xmlType = p.getChildNode(2);
    if (!SafeStrCmp(xmlType.getName(), "type"))
    {
      Assert(0);
      return;
    }
    *giveNum = ToInt(xmlGive.getText());
    *recvNum = ToInt(xmlRecv.getText());
    *tt = (TradeType)ToInt(xmlType.getText());
  }
}

MsgManager::MsgManager()
{
  m_newMsgs = 0;
}

void MsgManager::QueueMsg(const Msg& msg)
{
  if (m_msgsRecv.count(msg.m_id) > 0)
  {
//std::cout << "Discarding msg ID " << msg.m_id << " as already queued.\n";
    return; // already in set
  }
  m_msgsRecv.insert(msg.m_id);
  m_map.insert(std::make_pair(msg.m_whenSent, msg));
  m_newMsgs++;
}

const MsgManager::Msg* MsgManager::GetMsg(int msgId) const
{
  Assert(m_msgsRecv.count(msgId) > 0);
  for (Msgs::const_iterator it = m_map.begin(); it != m_map.end(); it++)
  {
    const Msg& msg = it->second;
    if (msg.m_id == msgId)
    {
      return &msg;
    }
  }
  return 0;
}

int MsgManager::HasNewMsgs() const
{
  return m_newMsgs;
}

void MsgManager::ResetNewMsgs()
{
  m_newMsgs = 0;
}

void MsgManager::CheckForNewMsgs()
{
  // Get Msgs with ID greater than last msg ID recved
  // Queue new msgs  -- in OnSuccess() in ReqGetNewMsgs
  SendGetNewMsgsReq();
}

void MsgManager::Update()
{
  // This erases messages, it's a temp queue??!!
  return; // TODO 


  // Check if any new msgs in queue
  if (m_map.empty())
  {
    return;
  }

  static ChatConsole* cc = TheChatConsole::Instance();
  static BroadcastConsole* bc = TheBroadcastConsole::Instance();

  // First look for broadcast msgs
  for (Msgs::iterator it = m_map.begin(); it != m_map.end(); )
  {
    Msg& msg = it->second;
    if (msg.m_recipId < 0)
    {
      // This is a broadcast msg
      // Discard msgs sent before today
      if (msg.m_whenSent < Time::Now().RoundDown(TimePeriod(ONE_DAY_IN_SECONDS)))
      {
        std::cout << "Discarding old broadcast msg: " << msg.m_text << "\n";
      }
      else
      {
        std::string name;
        std::string str;
        if (GetNameForPlayer(msg.m_senderId, &name))
        {
          str = name + ": ";
          str += msg.m_text; 
          bc->OnMsgRecv(str);
        }
        else
        {
          //std::cout << "Discarding broadcast msg (another room?) from player " 
          //  << msg.m_senderId << ": " << msg.m_text << "\n";
          str = msg.m_text; 
          bc->OnMsgRecv(str);
        }
      }
      m_map.erase(it++); // which doesn't invalidate iterator, right ??
    }
    else
    {
      ++it;
    }
  }

  // Show remaining msgs in chat console if allowed
  // Can we display the new msg ?
  if (cc->CanShowMsg())
  {
    // Tell the main game state that we have a new msg 
    Msgs::iterator it = m_map.begin();
    if (it != m_map.end())
    {
      Msg& msg = it->second;
      //gsm->ShowMsg(msg);

      if (msg.m_senderId < 0)
      {
        LurkMsg lm(msg.m_text, LURK_FG, LURK_BG, AMJU_CENTRE); 
        TheLurker::Instance()->Queue(lm);    
      }
      else
      {
        cc->ActivateChatRecv(true, &msg);
        cc->ActivateChatSend(true, msg.m_senderId);

        // Text to speech -- NB is here the best place ?
        TextToSpeech(msg.m_text);
      }

      // Mark message as read -- TODO only after player clicks OK in gui..?
      if (m_map.size() == 1)
      {
        // Last (i.e. most recent) msg - mark as read: this should 
        //  mark all earlier msgs as read also.
        MarkRead(msg);
      }

      m_map.erase(it);
    }
  }
}

void MsgManager::MarkRead(const Msg& msg)
{
  std::string url = TheVe1ReqManager::Instance()->MakeUrl(MARK_MSG_READ);
  url += "&id=";
  url += ToString(msg.m_id);
  TheVe1ReqManager::Instance()->AddReq(new ReqMsgRead(url), 1); // TODO only need 1 ??
}

void MsgManager::SendTradeMsg(
  int senderId, int recipId, const std::string& msg, 
  int giveNum, int recvNum, TradeType tt)
{
  std::string str = msg;
  AddTradeInfo(&str, giveNum, recvNum, tt);
  SendMsg(senderId, recipId, str);
}
 
void MsgManager::SendMsg(int senderId, int recipId, const std::string& msg)
{
  // How many msgs sent in total ? First message ? That's an achievement. etc.
  static const std::string NUM_MSGS_SENT = "num_msgs_sent";
  int numMsgsSent = 0;
  GetPlayerCount(NUM_MSGS_SENT, &numMsgsSent);
  ChangePlayerCount(NUM_MSGS_SENT, 1);

  numMsgsSent++; // just for convenience here, or we will be off by one below

  // NB Watch for race condition here. Multiple increments could batch up.
  if (senderId != SYSTEM_SENDER) // "system" msgs don't count
  {
    if (numMsgsSent >= 1 && !HasWonAchievement(ACH_MSGS_SENT_1))
    {
      OnWinAchievement(ACH_MSGS_SENT_1, "You sent a message!");
    }
    if (numMsgsSent >= 5 && !HasWonAchievement(ACH_MSGS_SENT_5))
    {
      OnWinAchievement(ACH_MSGS_SENT_5, "You sent your 5th message!");
    }
    if (numMsgsSent == 10 && !HasWonAchievement(ACH_MSGS_SENT_10))
    {
      OnWinAchievement(ACH_MSGS_SENT_10, "You sent your 10th message!");
    }
    if (numMsgsSent == 20 && !HasWonAchievement(ACH_MSGS_SENT_20))
    {
      OnWinAchievement(ACH_MSGS_SENT_20, "You sent your 20th message!");
    }
    if (numMsgsSent == 50 && !HasWonAchievement(ACH_MSGS_SENT_50))
    {
      OnWinAchievement(ACH_MSGS_SENT_50, "You sent your 50th message!!");
    }
  }
  Assert(senderId != -1);
  Assert(recipId != -1);
  // For now, assume senders and recips are Players
  /*
  if (senderId >= 0)
  {
    Assert(dynamic_cast<Player*>(TheGame::Instance()->GetGameObject(senderId).GetPtr()));
  }

  if (recipId >= 0)
  {
    Assert(dynamic_cast<Player*>(TheGame::Instance()->GetGameObject(recipId).GetPtr()));
  }
  */

  // Encode all characters in msg 
  std::string newmsg = EncodeMsg(msg); 

#ifdef SEND_DEBUG
std::cout << "Sending msg: to: " << recipId << " From: " << senderId << " msg: " << msg << "\n";
#endif

  static const int MAX_CONCURRENT_MSGS = 100; // ?

  std::string url = TheVe1ReqManager::Instance()->MakeUrl(SEND_MSG);
  url += "&recip=";
  url += ToString(recipId);
  url += "&sender=";
  url += ToString(senderId);
  url += "&msg='";
  url += newmsg;
  url += "'";
  TheVe1ReqManager::Instance()->AddReq(new ReqSendMsg(url), MAX_CONCURRENT_MSGS);
}

std::string EncodeMsg(const std::string& plainMsg)
{
  std::string result;

  int s = plainMsg.size();
  for (int i = 0; i < s; i++)
  {
    char c = plainMsg[i];
    result += ToHexString(c);
  }

  return result;
}

std::string DecodeMsg(const std::string& encodedMsg)
{
  std::string result;

  int s = encodedMsg.size();
  for (int i = 0; i < s; i += 2)
  {
    std::string hex = encodedMsg.substr(i, 2);
    int n = UIntFromHexString(hex);
    result += std::string(1, (char)n);
  }

  return result;
}

}

