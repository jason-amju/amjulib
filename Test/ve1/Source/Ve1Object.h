#ifndef VE1_OBJECT_H
#define VE1_OBJECT_H

#include <map>
#include <string>
#include <GameObject.h>
#include <Xml/XmlNodeInterface.h>
#include <GuiMenu.h>
#include <SceneNode.h>
#include <Shadow.h>

namespace Amju
{
typedef std::map<std::string, std::string> ValMap;

class Ve1Object : public GameObject
{
public:
  Ve1Object();

  virtual void Draw() { } // we use Scene Graph to draw

  virtual void Update(); // subclasses call this

  // Called when object is 'activated' because the local player changes location to 
  //  where this object is. 
  // ONLY CALL WHEN SAFE TO DO SO, I.E. ALL OBJECTS IN NEW LOCATION ARE CREATED....
  virtual void OnLocationEntry();

  // Called when this object leaves the location of the local player. 
  // This only applies to objects which can move autonomously, i.e. non-local players.
  virtual void OnLocationExit();

  const Vec3f& GetOldPos() const;
 

  // Set a state (key, val) pair
  virtual void SetKeyVal(const std::string& key, const std::string& val);
  bool Exists(const std::string& key) const;
  const std::string& GetVal(const std::string& key) const;
  ValMap* GetValMap();

  // Set the position - if a character, move to this pos over time, don't instantly set.
  // NB If location changes, we must call SetLocation().
  virtual void MoveTo(const Vec3f& pos);

  // Set in-game menu items appropriate to this object type
  virtual void SetMenu(GuiMenu*) { }

  // Set edit mode menu appropriate to this type: this is for edit mode only.
  // Call base class for menu choices common to all types
  virtual void SetEditMenu(GuiMenu*) = 0;

  // Rets true if location has been set for this object
  bool LocationReady() const;

  int GetLocation() const;

  // This updates the game state as we may enter or leave the same location as the local player.
  void SetLocation(int loc);

  bool IsSelected() const;
  void SetSelected(bool selected);

  void IgnorePortalId(int portalId);
  int GetIgnorePortalId() const;

  bool IsPickable() const;

  // Create a visible scene node for Edit mode
  void CreateEditNode();
 
  AABB* GetAABB();

protected:
  // Location ID. -1 means the object doesn't live in one particular location, it's in every location, 
  //  i.e. it's in whatever the current location is.
  int m_location;

  // Values set by SetKeyVal()
  ValMap m_valmap; 

  bool m_isSelected;

  int m_ignorePortalId;

  bool m_isPickable;

  Vec3f m_oldPos;

  RCPtr<SceneNode> m_sceneNode;
  RCPtr<Shadow> m_shadow;

  AABB m_aabb; 
};

// Keep track of keys used to set object properties
#define SET_KEY(x) x
}

#endif

