/*
    This file is part of KOrganizer.
    Copyright (c) 2003 Reinhold Kainhofer <reinhhold@kainhofer.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOINCIDENCETOOLTIP_H
#define KOINCIDENCETOOLTIP_H

#include <tqtooltip.h>

namespace KCal
{
class Calendar;
class Incidence;
}
using namespace KCal;

class KOAgendaItem;

/**
@author Reinhold Kainhofer
*/
class KOIncidenceToolTip : public QToolTip
{
  public:
    KOIncidenceToolTip( TQWidget *widget, Calendar *calendar, const TQDate &date, TQToolTipGroup *group = 0 )
      : TQToolTip (widget, group), mCalendar( calendar ), mDate( date ), mText(0) {}
/*    ~KOIncidenceToolTip();*/

  public:
    static void add ( TQWidget *widget, Calendar *calendar,
                      Incidence *incidence, const TQDate &date=TQDate(),
                      TQToolTipGroup *group = 0, const TQString &longText = "" );
    static void add( KOAgendaItem *item, Calendar *calendar,
                     Incidence *incidence = 0, const TQDate &date=TQDate(),
                     TQToolTipGroup *group = 0 );

    /* reimplmented from TQToolTip */
    void maybeTip( const TQPoint &pos );

  private:
    Calendar *mCalendar;
    TQDate mDate;
    TQString mText;
};

#endif
