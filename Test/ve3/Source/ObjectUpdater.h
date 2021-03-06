#ifndef OBJECT_UPDATER_H
#define OBJECT_UPDATER_H

#include <Singleton.h>
#include <map>
#include <Vec3.h>
#include "Timestamp.h"

namespace Amju
{
class ObjectUpdater : public NonCopyable
{
public:
  ObjectUpdater();
  ~ObjectUpdater();

  void Update();

  void SendUpdateReq(int objId, const std::string& key, const std::string& val);

  void SendPosUpdateReq(int objId, const Vec3f& val, int location);
  void SendChangeLocationReq(int objId, const Vec3f& val, int location);

  // Queue a position update for a game object. Called from position update successful response.
  void QueueUpdatePos(int id, const Vec3f& pos, int location);

  void QueueUpdate(int id, const std::string& key, const std::string& value);

  // Set from response
  void SetTimestampPos(Timestamp);
  void SetTimestampUpdate(Timestamp);

  void HintCheckForUpdates();

  // For Edit mode
  void Clear();

private:
  // Save object states to local cache. Save timestamp of last server response.
  bool Save();

  // Called on new session to load local cache, and last timestamp.
  bool Load();

private:
  struct PosLocation
  {
    PosLocation() : location(-1) { }
    PosLocation(const Vec3f& p, int loc) : pos(p), location(loc) { }

    Vec3f pos;
    int location;
  };

  typedef std::map<int, PosLocation> PosMap;
  PosMap m_posMap; 
  PosMap m_posMapCache;

  // Old: would accumulate many key/val pairs for each ID
//  typedef std::pair<std::string, std::string> KeyVal;
//  typedef std::map<int, KeyVal> KeyValMap;

  // Updates' primary key is <ID, Key> pair
  typedef std::pair<int, std::string> IdKey;

  typedef std::map<IdKey, std::string> KeyValMap;
  KeyValMap m_keyvalMap;
  KeyValMap m_keyvalMapCache;

  // Time of last response from server for the different query types. 
  Timestamp m_timestampPos;
  Timestamp m_timestampUpdate;

  bool m_isDirty; // ?
 
  // Count elapsed time, so we only query server periodically. This gets shortcut if we call a Hint* function 
  float m_updateElapsed; // for less frequent DB query
  float m_fastUpdateElapsed; // for more frequent poll of document

  float m_posElapsed;
};

typedef Singleton<ObjectUpdater> TheObjectUpdater;
}

#endif

