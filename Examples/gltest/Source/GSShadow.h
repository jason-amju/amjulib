#ifndef GS_SHADOW_H_INCLUDED
#define GS_SHADOW_H_INCLUDED

#include <Singleton.h>
#include "GSBase.h"

namespace Amju 
{
class GSShadow : public GSBase
{
  GSShadow();
  friend class Singleton<GSShadow>;

public:
  virtual void Update() override;
  virtual void DrawScene() override;
  virtual void OnActive() override;

};
typedef Singleton<GSShadow> TheGSShadow;
} // namespace
#endif
