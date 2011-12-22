#include "PlayerInfo.h"
#include <File.h>
#include <StringUtils.h>
#include <Directory.h>
#include <iostream>

namespace Amju
{
PlayerInfo::~PlayerInfo()
{
  Save();
}

void PlayerInfo::SetFilename(const std::string& filename)
{
  m_filename = filename;
}

void PlayerInfo::PISetString(const std::string& key, const std::string& val)
{
  m_map[key] = val;
}

void PlayerInfo::PISetBool(const std::string& key, bool val)
{
  m_map[key] = (val ? "1" : "0");
}

void PlayerInfo::PISetInt(const std::string& key, int val)
{
  m_map[key] = ToString(val);
}

void PlayerInfo::PISetFloat(const std::string& key, float val)
{
  m_map[key] = ToString(val);
}

const std::string& PlayerInfo::PIGetString(const std::string& key) const
{
  PIMap::const_iterator it = m_map.find(key);
  Assert(it != m_map.end());
  return it->second;
}

bool PlayerInfo::PIGetBool(const std::string& key) const
{
  PIMap::const_iterator it = m_map.find(key);
  if (it == m_map.end())
  {
    return false;
  }
  return (it->second == "1" ? true : false); 
}

int PlayerInfo::PIGetInt(const std::string& key) const
{
  PIMap::const_iterator it = m_map.find(key);
  Assert(it != m_map.end());
  return ToInt(it->second);
}

float PlayerInfo::PIGetFloat(const std::string& key) const
{
  PIMap::const_iterator it = m_map.find(key);
  Assert(it != m_map.end());
  return ToFloat(it->second);
}

bool PlayerInfo::Load()
{
  if (!FileExists(File::GetRoot() + m_filename))
  {
std::cout << "Loading player info " << m_filename << " - it's a new file\n";

    // We assume the player is new and has not saved any player info yet - this is OK.
    return true;
  }

std::cout << "Loading player info " << m_filename << "...\n";
  File f;
  if (!f.OpenRead(m_filename))
  {
    return false;
  }
  int num = 0;
  if (!f.GetInteger(&num))
  {
    return false;
  }
  for (int i = 0; i < num; i++)
  {
    std::string s;
    if (!f.GetDataLine(&s))
    {
      return false;
    }

std::cout << " Got line: " << s << "\n";

    Strings strs = Split(s, ',');
    if (strs.size() != 2)
    {
      f.ReportError("Bad line in player info: " + s);
      return false;
    }
    PISetString(strs[0], strs[1]);
  }
std::cout << "End of player info load.\n";

  return true;
}

bool PlayerInfo::Save()
{
std::cout << "SAVING PLAYER INFO FILE " << m_filename << "\n";

  File f; // TODO no glue file
  if (!f.OpenWrite(m_filename))
  {
    return false;
  }
  f.WriteInteger(m_map.size());
  for (PIMap::iterator it = m_map.begin(); it != m_map.end(); ++it)
  {
    // TODO what about commas in keys or values ?!
    f.Write(it->first + std::string(",") + it->second);
  }

  return true;
}


PlayerInfoManager::PlayerInfoManager()
{
  m_currentPI = 0;
}

PlayerInfo* PlayerInfoManager::GetPI()
{
  Assert(m_currentPI);
  return m_currentPI;
}

void PlayerInfoManager::SetCurrentPlayer(const std::string& filename)
{
  PIMap::iterator it = m_map.find(filename);
  if (it == m_map.end())
  {
    PlayerInfo* pi = new PlayerInfo;
    pi->SetFilename(filename);
    if (!pi->Load())
    {
      // TODO What to do here ?? Player has lost player data!! Oh no!
    }
    m_map[filename] = pi;
    m_currentPI = pi;
  }
  else
  {
    m_currentPI = it->second;
  }
}
}

