#ifndef FOOD_H_INCLUDED
#define FOOD_H_INCLUDED

#include "Furniture.h"

namespace Amju 
{
void SetRandomFoodInLocation();

class FoodManager : public Ve1Object
{
public:
  static const char* TYPENAME;

  FoodManager();
  virtual const char* GetTypeName() const;

  // Create Food cells in new location, local to client only
  virtual void OnLocationEntry();
  virtual void SetEditMenu(GuiMenu*) {}
  virtual void Update();
  virtual bool Load(File*);

private:
  // Put m_numfood foods in the room, around m_pos, in a circle of size m_radius
  int m_numfood;
  float m_radius;
};

class Food : public Furniture
{
public:
  static const char* TYPENAME;
  virtual const char* GetTypeName() const;

  Food();

  virtual bool Load(File*);
  virtual void SetEditMenu(GuiMenu*) {}
  virtual void SetMenu(GuiMenu*) {}
  virtual void Update();

  void SetOwner(Ve1Object* v) { m_owner = v; }
  Ve1Object* GetOwner() { return m_owner; }

private:
  Ve1Object* m_owner;
};
} // namespace
#endif
