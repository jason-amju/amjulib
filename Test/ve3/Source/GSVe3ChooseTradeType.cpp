#include <Game.h>
#include <CursorManager.h>
#include "GSVe3ChooseTradeType.h"
#include "GSVe3MakeTradeRequest.h"
#include "LurkMsg.h"
#include "LayerGroup.h"
#include "LocalPlayer.h"
#include "GameConsts.h"

namespace Amju
{
static void OnBack(GuiElement*)
{
  TheGSVe3ChooseTradeType::Instance()->GoBack();
}

static void OnGiveFood(GuiElement*)
{
  TheGSVe3ChooseTradeType::Instance()->OnGiveFood();
}

static void OnGiveTreasure(GuiElement*)
{
  TheGSVe3ChooseTradeType::Instance()->OnGiveTreasure();
}

GSVe3ChooseTradeType::GSVe3ChooseTradeType()
{
  m_showLurk = true;
}

void GSVe3ChooseTradeType::Update()
{
  GSGui::Update();

  // TODO 2 animated player sprites  
  //m_spriteNode.Update();

  TheLurker::Instance()->Update();
}

void GSVe3ChooseTradeType::Draw()
{
  GSGui::Draw();
}

void GSVe3ChooseTradeType::Draw2d()
{
  GSGui::Draw2d();
  // TODO

  Vec2f TRANSLATE[4] = { Vec2f(-25, 0), Vec2f(75.0f, 0), Vec2f(175, 0), Vec2f(275, 0) };

  // Draw players
  AmjuGL::PushMatrix();

  AmjuGL::RotateX(90.0f); 
  AmjuGL::Translate(-0.6f, 0, -0.3f);
  AmjuGL::Scale(0.005f, 1, 0.005f);

  for (int i = 0; i < 4; i++)
  {
    AmjuGL::PushMatrix();
    AmjuGL::Translate(TRANSLATE[i].x, TRANSLATE[i].y, 0);
    m_spriteNodes[i].Draw();
    AmjuGL::PopMatrix();

    m_spriteNodes[i].Update();
  }
  AmjuGL::PopMatrix();

  TheCursorManager::Instance()->Draw();
}

void GSVe3ChooseTradeType::OnActive()
{
  GSGui::OnActive();

  m_gui = LoadGui("gui-ve3-choosetradetype.txt");
  Assert(m_gui);

  GetElementByName(m_gui, "back-button")->SetCommand(OnBack);
  GetElementByName(m_gui, "give-treasure-button")->SetCommand(Amju::OnGiveTreasure);
  GetElementByName(m_gui, "give-food-button")->SetCommand(Amju::OnGiveFood);

  Assert(m_player); // the other player
  Player* localPlayer = GetLocalPlayer();
  Assert(localPlayer);
  Player* PLAYER[4] = { localPlayer, m_player, localPlayer, m_player };
  for (int i = 0; i < 4; i++)
  {
    LayerGroups layerGroups; // store settings for each layer
    layerGroups.SetFromSprite(PLAYER[i]->GetSprite());
    layerGroups.SetSprite(&m_spriteNodes[i].GetSprite());

    m_spriteNodes[i].Update();
  }

  SetHomeButton();

  // Set food and treasure counts for both players
  Assert(dynamic_cast<GuiText*>(GetElementByName(m_gui, "food-you-text")));
  Assert(dynamic_cast<GuiText*>(GetElementByName(m_gui, "food-other-text")));
  Assert(dynamic_cast<GuiText*>(GetElementByName(m_gui, "treasure-you-text")));
  Assert(dynamic_cast<GuiText*>(GetElementByName(m_gui, "treasure-other-text")));

  if (localPlayer->Exists(FOOD_STORED_KEY))
  {
    ((GuiText*)GetElementByName(m_gui, "food-you-text"))->SetText(localPlayer->GetVal(FOOD_STORED_KEY));
  }

  if (m_player->Exists(FOOD_STORED_KEY))
  {
    ((GuiText*)GetElementByName(m_gui, "food-other-text"))->SetText(m_player->GetVal(FOOD_STORED_KEY));
  }

  if (localPlayer->Exists(TREASURE_KEY))
  {
    ((GuiText*)GetElementByName(m_gui, "treasure-you-text"))->SetText(localPlayer->GetVal(TREASURE_KEY));
  }

  if (m_player->Exists(TREASURE_KEY))
  {
    ((GuiText*)GetElementByName(m_gui, "treasure-other-text"))->SetText(m_player->GetVal(TREASURE_KEY));
  }
}

void GSVe3ChooseTradeType::OnGiveFood()
{
  // New trading mechanic: send a message to trade food for treasure
  GSVe3MakeTradeRequest* mtr = TheGSVe3MakeTradeRequest::Instance();
  mtr->Reset();
  mtr->SetPrevState(this);
  mtr->SetOtherPlayer(m_player->GetId());
  // Set up trade type
  mtr->SetTradeType(TRADE_FOOD_FOR_TREASURE);
  TheGame::Instance()->SetCurrentState(mtr);

  /*
  // Check we have some
  Player* p = GetLocalPlayer();
  Assert(p);
  if (p->Exists(FOOD_STORED_KEY))  
  {
    int food = ToInt(p->GetVal(FOOD_STORED_KEY));
    if (food < 1)
    {
      // Show lurk msg - can't give food!
      LurkMsg lm("You don't have any food to give!", LURK_FG, LURK_BG, AMJU_CENTRE); 
      TheLurker::Instance()->Queue(lm);    
    }
    else
    {
      // Decremenet local player food, add one to the other player's food count
      ChangePlayerCount(FOOD_STORED_KEY, -1);
      ChangeObjCount(m_player->GetId(), FOOD_STORED_KEY, +1);

      // Also change lifetime counters
      ChangePlayerCount(FOOD_GIVEN_KEY, +1);
      ChangeObjCount(m_player->GetId(), FOOD_RECEIVED_KEY, +1);

      ShowPlayer(m_player, m_gui); // to update GUI

      TheSoundManager::Instance()->PlayWav("sound/kiss.wav");

      std::string str = "You gave some food to " + m_player->GetName() + "!";
      LurkMsg lm(str, LURK_FG, LURK_BG, AMJU_CENTRE); 
      TheLurker::Instance()->Queue(lm);    

      // Add a message (to guestbook?) so player will know what happened
      std::string msg = p->GetName() + " gave some food to " + m_player->GetName() + "!";
      MsgManager* mm = TheMsgManager::Instance();
      mm->SendMsg(p->GetId(), m_player->GetId(), msg);
      // Also a message in our own guestbook?
      mm->SendMsg(m_player->GetId(), p->GetId(), str);
      // TODO Get this msg into guestbook - send a request for new msgs
    }
  }
  */
}

void GSVe3ChooseTradeType::OnGiveTreasure()
{
  // New trading mechanic: send a message to trade food for treasure
  GSVe3MakeTradeRequest* mtr = TheGSVe3MakeTradeRequest::Instance();
  mtr->Reset();
  mtr->SetPrevState(this);
  mtr->SetOtherPlayer(m_player->GetId());
  // Set up trade type
  mtr->SetTradeType(TRADE_TREASURE_FOR_FOOD);
  TheGame::Instance()->SetCurrentState(mtr);

  /*
  // Check we have some
  Player* p = GetLocalPlayer();
  Assert(p);
  if (p->Exists(TREASURE_KEY))  
  {
    int tr = ToInt(p->GetVal(TREASURE_KEY));
    if (tr < 1)
    {
      LurkMsg lm("You don't have any treasure to give!", LURK_FG, LURK_BG, AMJU_CENTRE); 
      TheLurker::Instance()->Queue(lm);    
    }
    else
    {
      // Decremenet local player treasure, add one to this other player's treasure
      ChangePlayerCount(TREASURE_KEY, -1);
      ChangeObjCount(m_player->GetId(), TREASURE_KEY, +1);

      // Also change lifetime counters
      ChangePlayerCount(TREASURE_GIVEN_KEY, +1);
      ChangeObjCount(m_player->GetId(), TREASURE_RECEIVED_KEY, +1);

      ShowPlayer(m_player, m_gui); // to update GUI

      TheSoundManager::Instance()->PlayWav("sound/cashreg.wav");

      std::string str = "You gave some treasure to " + m_player->GetName() + "!";
      LurkMsg lm(str, LURK_FG, LURK_BG, AMJU_CENTRE); 
      TheLurker::Instance()->Queue(lm);    

      // Add a message (to guestbook?) so player will know what happened
      std::string msg = p->GetName() + " gave some treasure to " + m_player->GetName() + "!";
      MsgManager* mm = TheMsgManager::Instance();
      mm->SendMsg(p->GetId(), m_player->GetId(), msg);
      // Also a message in our own guestbook?
      mm->SendMsg(m_player->GetId(), p->GetId(), str);
      // TODO Get this msg into guestbook - send a request for new msgs
    }
  }
  */
}


}
