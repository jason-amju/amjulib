#ifndef AMJU_GAME_H
#define AMJU_GAME_H

#include "Singleton.h"
#include "GameState.h"
#include "GameObject.h"
#include "EventListener.h"
#include "Colour.h"

namespace Amju
{
class Game : public NonCopyable, public EventListener
{
private:
  Game();
  friend class Singleton<Game>;

public:
  void Update();
  void Draw();

  // Loop forever until quit event received
  void Run();

  // Execute one game loop, use if Run() not suitable for platform
  void RunOneLoop();

  // Event Listener
  virtual bool OnQuitEvent();

  // Game States
  GameState* GetState(); // Get current state
  void SetCurrentState(GameState* gs);

  // Game Objects
  typedef std::map<int, PGameObject> GameObjects;

  PGameObject GetGameObject(int id);
  void AddGameObject(PGameObject object);
  void EraseGameObject(int id);
  void ClearGameObjects(); // erase all
  GameObjects* GetGameObjects();

  // Functions commonly used by Game States
  void UpdateGameObjects();

  void SetClearColour(const Colour&);

private:
  void UpdateState(); // go to new state if set

private:
  GameState* m_currentState;
  GameState* m_newState;

  GameObjects m_objects;

  Colour m_clearColour;
};
typedef Singleton<Game> TheGame;
}

#endif
