#ifndef GS_LIGHTING_H_INCLUDED
#define GS_LIGHTING_H_INCLUDED

#include <Singleton.h>
#include <GameState.h>

namespace Amju 
{
class GSLighting : public GameState
{
  GSLighting();
  friend class Singleton<GSLighting>;

public:
  virtual void Update();
  virtual void Draw();
  virtual void Draw2d();
  virtual void OnActive();

  virtual bool OnCursorEvent(const CursorEvent&);
  virtual bool OnMouseButtonEvent(const MouseButtonEvent&);
  virtual bool OnButtonEvent(const ButtonEvent& be);
};
typedef Singleton<GSLighting> TheGSLighting;
} // namespace
#endif
