#include <iostream>
#include <AmjuFirst.h>
#include <TimePeriod.h>
#include "ResearchCalendar.h"
#include <AmjuFinal.h>

namespace Amju
{
std::ostream& operator<<(std::ostream& os, const ResearchDate& rd)
{
  return os 
    << rd.m_time.ToStringJustDate()
    << ": multi: "
    << rd.m_playMulti
    << " single: " 
    << rd.m_playSingle
    << " cog test: " 
    << rd.m_cogTest;
}

void ResearchCalendar::Clear()
{
  m_dates.clear();
}

const ResearchDates& ResearchCalendar::GetResearchDates() const
{
  return m_dates;
}

void ResearchCalendar::AddResearchDate(ResearchDate* d)
{
  ResearchDates::iterator it = m_dates.find(d->m_time);
  
  if (it == m_dates.end())
  {
    m_dates[d->m_time] = d;
  }
  else
  {
#ifdef _DEBUG
    std::cout << "Duplicated research date: " << *d << "\n";
#endif
    if (d->m_cogTest)
    {
      it->second->m_cogTest = true;
    }
    if (d->m_playMulti)
    {
      it->second->m_playMulti = true;
    }
    if (d->m_playSingle)
    {
      it->second->m_playSingle = true;
    }
#ifdef _DEBUG
    std::cout << "  now it's: " << *(it->second) << "\n";
#endif
  }
}

const ResearchDate* ResearchCalendar::GetToday() const
{
  Time today(Time::Now());
  today.RoundDown(TimePeriod(ONE_DAY_IN_SECONDS));

  for (ResearchDates::const_iterator it = m_dates.begin(); it != m_dates.end(); ++it)
  {
    if (it->first == today)
    {
      return it->second;
    }
  }

  return 0;
}

const ResearchDate* ResearchCalendar::GetNext() const
{
  return 0;
}

}


