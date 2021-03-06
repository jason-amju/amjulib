#ifndef GS_INITIALISE_H_INCLUDED
#define GS_INITIALISE_H_INCLUDED

#include <Singleton.h>
#include "GSBase.h"

namespace Amju 
{
class GSInitialise : public GSBase
{
  GSInitialise();
  friend class Singleton<GSInitialise>;

public:
  virtual void Update() override;
  virtual void DrawScene() override;
  virtual void OnActive() override;

  virtual bool OnCursorEvent(const CursorEvent&) override;
  virtual bool OnMouseButtonEvent(const MouseButtonEvent&) override;
  
  virtual void CreateTweakMenu() override {}
};
typedef Singleton<GSInitialise> TheGSInitialise;
} // namespace
#endif
