#ifndef OBJECT_MANAGER_H_INCLUDED
#define OBJECT_MANAGER_H_INCLUDED

#include <Singleton.h>
#include <string>
#include <set>
#include <RCPtr.h>
#include <vector>
#include <map>
#include "DownloadManager.h"

namespace Amju
{
// Info about an object we need to create.
struct Object : public RefCounted
{
  int m_id;
  int m_owner;
  std::string m_type;
  std::string m_assetlist;
  std::string m_datafile;
 
  bool m_loaded;

  Object(int id, int owner, const std::string& type, const std::string& assetlist, const std::string& datafile) :
    m_id(id), m_owner(owner), m_type(type), m_assetlist(assetlist), m_datafile(datafile), 
    //m_datafileLocal(false), m_assetsLocal(false),
    m_loaded(false)
  {}  

  void Load();
};

typedef RCPtr<Object> PObject;

inline bool operator<(const PObject& o1, const PObject& o2)
{
  if (o1->m_id < o2->m_id)
  {
    return true;
  }
  else if (o1->m_id > o2->m_id)
  {
    return false;
  }

  if (o1->m_owner < o2->m_owner)
  {
    return true;
  }
  else if (o1->m_owner > o2->m_owner)
  {
    return false;
  }

  if (o1->m_type < o2->m_type)
  {
    return true;
  }
  else if (o1->m_type > o2->m_type)
  {
    return false;
  }

  if (o1->m_assetlist < o2->m_assetlist)
  {
    return true;
  }
  /*
  else if (o1->m_assetlist > o2->m_assetlist)
  {
    return false;
  }
  */

  return false;
}


struct AssetList : public RefCounted
{
  std::string m_name; 

  // State of asset list, progresses through these stages but can skip forward.
  enum State
  {
    AMJU_AL_UNKNOWN,
    AMJU_AL_LOADING,
    AMJU_AL_ALL_ASSETS_LOADED
  };

  State m_state;

  void Update();

  bool AllAssetsLoaded() const { return m_state == AMJU_AL_ALL_ASSETS_LOADED; }

  typedef std::vector<std::string> AssetNames;
  AssetNames m_assetNames;

  AssetList(const std::string& name) : 
    m_name(name), m_state(AMJU_AL_UNKNOWN) {}

  // Load contents once file is local
  bool Load();
};

typedef RCPtr<AssetList> PAssetList;


// Periodically checks for new objects (created since last check).
// (If first time this process, timestamp is set so we get all objects.)
// For new objects, download the asset list. Then download any assets not on local disk.
// When all assets are downloaded, create the object locally.
class ObjectManager : public DownloadManager
{
public:
  ObjectManager();

  // Called every frame
  void Update();

  void AddObject(PObject);

private:
  float m_elapsed;

  // Asset list:
  // Objects point to asset lists.
  // Asset lists point to individual assets. We want asset lists to be updated when an asset gets downloaded.
  // When an asset list is completely downloaded we can create all the objects with that asset list.

  typedef std::set<PObject> Objects;
  Objects m_objects; // objects which we have, or are in the process of downloading/creating.

  typedef std::map<std::string, PAssetList> AssetLists;
  AssetLists m_assetLists;
};

typedef Singleton<ObjectManager> TheObjectManager;
}

#endif

