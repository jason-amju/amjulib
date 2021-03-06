#include <AmjuFirst.h>
#include <iostream>
#include <AmjuAssert.h>
#include <Timer.h>
#include <Game.h>
#include "EventPoller.h"
#include "GameStateListener.h"
#include <AmjuFinal.h>

//#define EP_DEBUG

namespace Amju
{
static float s_cursorScaleX = 1;
static float s_cursorScaleY = 1;
static float s_cursorTranslateX = 0;
static float s_cursorTranslateY = 0;

EventPollerImpl* EventPoller::GetImpl()
{
  Assert(m_pImpl.GetPtr());
  return m_pImpl;
}

void EventPollerImpl::Update(Listeners* pListeners)
{
  while (!m_q.empty())
  {
    Event* event = m_q.front(); // Events on heap
    m_q.pop();

    NotifyListenersWithPriority(event, pListeners);

    delete event; // Events on heap
  }
}

void EventPollerImpl::QueueEvent(Event* event)
{
  CursorEvent* ce = dynamic_cast<CursorEvent*>(event);

  // Work out mouse/cursor dx/dy
  if (ce)
  {
    ce->x = ce->x * s_cursorScaleX + s_cursorTranslateX;
    ce->y = ce->y * s_cursorScaleY + s_cursorTranslateY;

    if (ce->controller == 0)
    {
      static Vec2f prev(ce->x, ce->y);
      ce->dx = ce->x - prev.x;
      ce->dy = ce->y - prev.y;
      prev = Vec2f(ce->x, ce->y);
    }
    else if (ce->controller == 1)
    {
      static Vec2f prev(ce->x, ce->y);
      ce->dx = ce->x - prev.x;
      ce->dy = ce->y - prev.y;
      prev = Vec2f(ce->x, ce->y);
    }
    else if (ce->controller == 2)
    {
      static Vec2f prev(ce->x, ce->y);
      ce->dx = ce->x - prev.x;
      ce->dy = ce->y - prev.y;
      prev = Vec2f(ce->x, ce->y);
    }
    else if (ce->controller == 3)
    {
      static Vec2f prev(ce->x, ce->y);
      ce->dx = ce->x - prev.x;
      ce->dy = ce->y - prev.y;
      prev = Vec2f(ce->x, ce->y);
    }
  }

  m_q.push(event);
}

static KeyEvent lastKeyEvent;
static const float AUTO_REPEAT_START_DELAY = 0.5f; // TODO CONFIG 
static const float AUTO_REPEAT_RPT_DELAY = 0.1f;

static float rptTimer = 0;

enum AutoRepeatState { AMJU_AR_NONE, AMJU_AR_START_WAIT, AMJU_AR_RPT_WAIT }; 
static AutoRepeatState arState = AMJU_AR_NONE;

void HandleKey(KeyEvent* ke)
{
  if (ke->keyDown)
  {
    if (arState == AMJU_AR_NONE)
    {
      lastKeyEvent = *ke;
      arState = AMJU_AR_START_WAIT;
      rptTimer = AUTO_REPEAT_START_DELAY;
    }
  }
  else
  {
    arState = AMJU_AR_NONE;
    rptTimer = 0;
  }
}

void Repeat()
{
  if (arState == AMJU_AR_NONE)
  {
    return;
  }

  rptTimer -= TheTimer::Instance()->GetDt();

  if (rptTimer < 0)
  {
    // Have reached time limit, we now start repeating
    // TODO Queue Key Up event first ???
    TheEventPoller::Instance()->GetImpl()->QueueEvent(new KeyEvent(lastKeyEvent));

    arState = AMJU_AR_RPT_WAIT;
    rptTimer = AUTO_REPEAT_RPT_DELAY;
  }
}

void HandleDoubleClick(MouseButtonEvent* mbe)
{
  static const float DOUBLE_CLICK_TIME = 0.5f; // TODO CONFIG

  // For the clicked button, if time since last down click is below threshold, generate a
  //  double click event.

  // Time since last click for each button - start values prevent first
  //  click from being treated as double-click
  static float time[] = {-DOUBLE_CLICK_TIME, -DOUBLE_CLICK_TIME, -DOUBLE_CLICK_TIME }; 

  if (mbe->isDown)
  {
    float timeNow = TheTimer::Instance()->GetElapsedTime();
    float dt = timeNow - time[(int)mbe->button];

//std::cout << "Double click: dt=" << dt << "\n";

    if (dt < DOUBLE_CLICK_TIME)
    {
      DoubleClickEvent* dce = new DoubleClickEvent;
      dce->button = mbe->button;
      dce->x = mbe->x;
      dce->y = mbe->y;
      TheEventPoller::Instance()->GetImpl()->QueueEvent(dce);
    }
    time[(int)mbe->button] = timeNow;
  }
}

void EventPollerImpl::NotifyListenersWithPriority(Event* event, Listeners* pListeners)
{
  if (dynamic_cast<KeyEvent*>(event)) // TODO Auto-repeatable flag ?
  {
    HandleKey((KeyEvent*)event);
  }

  if (dynamic_cast<MouseButtonEvent*>(event))
  {
    HandleDoubleClick((MouseButtonEvent*)event);
  }

  int eaten = AMJU_MAX_PRIORITY + 1;

  // For debug reporting  
  EventListener* prevlistener = nullptr;

  for (Listeners::iterator it = pListeners->begin(); it != pListeners->end(); ++it)
  {
    if (it->first > eaten)
    {
      // This listener has a lower priority than a listener which ate the event
#ifdef EP_DEBUG
Assert(prevlistener);
std::cout << "Event eaten by listener " << typeid(*prevlistener).name() << ".\n";
#endif
      break;
    }

    EventListener* listener = it->second;
    Assert(listener);
    prevlistener = listener;

    if (event->UpdateListener(listener))
    {
      // Eaten
      eaten = it->first;
    }
  }
}

void EventPoller::Clear()
{
  m_listeners.clear();
}

void EventPoller::AddListener(EventListener* pListener, int priority)
{
  Assert(pListener);
  Assert(priority >= AMJU_MIN_PRIORITY);
  Assert(priority <= AMJU_MAX_PRIORITY);
  m_listeners.insert(std::make_pair(priority, pListener));
}

void EventPoller::RemoveListener(EventListener* pListener)
{
  for (Listeners::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
  {
    if (it->second == pListener)
    {
      m_listeners.erase(it);
      return;
    }
  }
  //Assert(0); // listener not found
}

bool EventPoller::HasListener(EventListener* pListener) const
{
  for (Listeners::const_iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
  {
    if (it->second == pListener)
    {
      return true;
    }
  }
  return false;
}

void EventPoller::SetImpl(EventPollerImpl* pImpl)
{
  m_pImpl = pImpl;
}

void EventPoller::AddCursorListener(EventListener* cursor)
{
  static const int CURSOR_PRIORITY = -2; // highest, so always responds
  m_cursors.insert(std::make_pair(CURSOR_PRIORITY, cursor));
}

void EventPoller::SetModalListener(EventListener* listener)
{
  if (m_modalListener == listener)
  {
    return;
  }

#ifdef _DEBUG
  std::cout << "Event Poller modal listener ";
  if (listener)
  {
    std::cout << " SET to: " << typeid(listener).name() << "\n";
  }
  else
  {
    std::cout << "RESET\n";
  }
#endif
  m_modalListener = listener;
}

void EventPoller::Update()
{
  Assert(m_pImpl.GetPtr());

  if (m_modalListener)
  {
    Listeners listeners = m_cursors;
    // the only listener to get events, so priority doesn't matter
    listeners.insert(std::make_pair(0, m_modalListener)); 

    m_pImpl->Update(&listeners);
  }
  else
  {
    // Copy listeners so we can add or remove listeners in response to an event
    Listeners copyListeners = m_listeners;
    copyListeners.insert(m_cursors.begin(), m_cursors.end());
    m_pImpl->Update(&copyListeners);
  }

  Repeat();
}

void EventPoller::SetCursorTransform(float scaleX, float scaleY, float translateX, float translateY)
{
  s_cursorScaleX = scaleX;
  s_cursorScaleY = scaleY;
  s_cursorTranslateX = translateX;
  s_cursorTranslateY = translateY;
}

}
