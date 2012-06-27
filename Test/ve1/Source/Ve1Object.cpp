#include "Ve1Object.h"
#include "LocalPlayer.h"
#include "ObjectManager.h"

namespace Amju
{
Ve1Object::Ve1Object() : m_location(-1)
{
  m_isSelected = false;
  m_ignorePortalId = -1;
  m_isPickable = true;
}

void Ve1Object::Update()
{
  m_oldPos = m_pos;
}

const Vec3f& Ve1Object::GetOldPos() const
{
  return m_oldPos;
}

bool Ve1Object::IsPickable() const 
{ 
  return m_isPickable; 
}

void Ve1Object::IgnorePortalId(int portalId)
{
  m_ignorePortalId = portalId;
}

int Ve1Object::GetIgnorePortalId() const
{
  return m_ignorePortalId;
}

bool Ve1Object::IsSelected() const
{
  return m_isSelected;
}

void Ve1Object::SetSelected(bool selected)
{
  m_isSelected = selected;
}

int Ve1Object::GetLocation() const
{
#ifdef _DEBUG
  if (m_location == -1)
  {
    std::cout << "Warning, object " << GetId() << " has location -1.\n"; 
  }
#endif

  //Assert(m_location != -1);

  return m_location;
}

void Ve1Object::SetLocation(int newLocation)
{
std::cout << "Setting location for " << m_id << " (" << GetTypeName() << "): " << newLocation << " (old: " << m_location << ")...\n";

  // Remove from game & scene graph etc if not in same location as local player
  // BUT object could also Enter the local player location!

  int oldLocation = m_location;

  if (oldLocation == newLocation)
  {
    return;
  }

  m_location = newLocation;

  int localPlayerLoc = GetLocalPlayerLocation();
  
  if (oldLocation == localPlayerLoc && newLocation != localPlayerLoc)
  {
    // We have just exited the local player location
    //OnLocationExit();  // ObjectManager will call, right ?
    // TODO notify ObjectManager ?
    TheObjectManager::Instance()->OnObjectChangeLocation(m_id); 
  }
  else if (oldLocation != localPlayerLoc && newLocation == localPlayerLoc)
  {
    // We have just entered the local player location
    //OnLocationEntry();
    TheObjectManager::Instance()->OnObjectChangeLocation(m_id); 
  }

  // NB What about when an object gets a Move or other change to state, and is not in the local
  //  player location...??!?!
  // It still gets updates if it exists in memory (i.e. ObjectManager has created it)
}

void Ve1Object::MoveTo(const Vec3f& pos)
{
  // Default behaviour: instantly change position
  SetPos(pos);
}

void Ve1Object::SetKeyVal(const std::string& key, const std::string& val)
{
  m_valmap[key] = val; 
}

bool Ve1Object::Exists(const std::string& key) const
{
  return (m_valmap.find(key) != m_valmap.end());
}

const std::string& Ve1Object::GetVal(const std::string& key) const
{
  Assert(Exists(key));
  ValMap::const_iterator it = m_valmap.find(key);
  Assert(it != m_valmap.end());
  return it->second;
}

ValMap* Ve1Object::GetValMap()
{
  return &m_valmap;
}

}

