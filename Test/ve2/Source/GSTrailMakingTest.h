#ifndef GS_TRAIL_MAKING_TEST_H_INCLUDED
#define GS_TRAIL_MAKING_TEST_H_INCLUDED

#include <vector>
#include <Vec2.h>
#include <StringUtils.h>
#include <Singleton.h>
#include "GSCogTestBase.h"

namespace Amju
{
class GSTrailMakingTest;
typedef Singleton<GSTrailMakingTest> TheGSTrailMakingTest;

struct TrailCircle
{
  TrailCircle(const Vec2f& pos, const std::string& str) : m_pos(pos), m_str(str), m_clicked(false) {}

  void Draw();
  bool IsClicked(const Vec2f& cursorPos);

  Vec2f m_pos;
  std::string m_str; // 1, A, 2, B... etc
  bool m_clicked;
};

class TrailListener;

class GSTrailMakingTest : public GSCogTestBase
{
  GSTrailMakingTest();
  friend class Singleton<GSTrailMakingTest>;

public:
  virtual void OnActive();
  virtual void OnDeactive();
  virtual void Update();
  virtual void Draw();
  virtual void Draw2d();

  virtual bool OnCursorEvent(const CursorEvent& ce);
  virtual bool OnMouseButtonEvent(const MouseButtonEvent& mbe);

protected:
  typedef std::vector<TrailCircle> Circles;
  Circles m_circles;
 
  int m_currentCircle;
  int m_correct;

  TrailListener* m_listener;
};
}

#endif
