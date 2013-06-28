#include "LoadLevel.h"
#include <Game.h>
#include <GameObjectFactory.h>
#include <File.h>

namespace Amju
{
bool LoadLevel(const std::string& filename)
{
  File f;
  if (!f.OpenRead(filename))
  {
    return false;
  }
  std::string str;
  while (f.GetDataLine(&str))
  {
    GameObject* go = TheGameObjectFactory::Instance()->Create(str);
    if (!go)
    {
      f.ReportError("I don't know how to create this: " + str);
      return false;
    }
    if (!go->Load(&f))
    {
      f.ReportError("Failed to load game object " + str);
      return false;
    }
    TheGame::Instance()->AddGameObject(go);
  }
  return true;
}

}
