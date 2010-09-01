/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KORG_TIMELINEITEM_H
#define KORG_TIMELINEITEM_H

#define private protected
#include <kdgantt/KDGanttViewTaskItem.h>
#undef protected

#include <tqmap.h>
#include <tqvaluelist.h>

class KDGanttView;
class KDCanvasPolygon;

namespace KCal {
  class Calendar;
  class ResourceCalendar;
  class Incidence;
}

namespace KOrg {

class TimelineSubItem;

class TimelineItem : public KDGanttViewTaskItem
{
  public:
    TimelineItem( const TQString &label, KCal::Calendar *calendar, KDGanttView* parent );

    void insertIncidence( KCal::Incidence *incidence,
                          const TQDateTime &start = TQDateTime(),
                          const TQDateTime &end = TQDateTime() );
    void removeIncidence( KCal::Incidence *incidence );

    void moveItems( KCal::Incidence* incidence, int delta, int duration );

  private:
    KCal::Calendar *mCalendar;
    TQMap<KCal::Incidence*, TQValueList<TimelineSubItem*> > mItemMap;
};

class TimelineSubItem : public KDGanttViewTaskItem
{
  public:
    TimelineSubItem( KCal::Calendar *calendar, KCal::Incidence *incidence, TimelineItem *parent );
    ~TimelineSubItem();

    KCal::Incidence* incidence() const { return mIncidence; }

    TQDateTime originalStart() const { return mStart; }
    void setOriginalStart( const TQDateTime &dt ) { mStart = dt; }

  private:
    void showItem( bool show = true, int coordY = 0 );

  private:
    KCal::Incidence *mIncidence;
    TQDateTime mStart;
    KDCanvasPolygon *mLeft, *mRight;
    int mMarkerWidth;
};

}

#endif
