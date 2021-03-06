#ifndef GS_CLIPTEST_H_INCLUDED
#define GS_CLIPTEST_H_INCLUDED

#include <Singleton.h>
#include "GSBase.h"

namespace Amju 
{
class GSClipTest : public GSBase
{
  GSClipTest();
  friend class Singleton<GSClipTest>;

public:
  virtual void Update();
  virtual void Draw();
  virtual void Draw2d();
  virtual void OnActive();

  virtual bool OnCursorEvent(const CursorEvent&);
  virtual bool OnMouseButtonEvent(const MouseButtonEvent&);
};
typedef Singleton<GSClipTest> TheGSClipTest;
} // namespace
#endif
